/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#import <zlib.h>

#import "OOoSpotlightAndQuickLookImporter.h"
#import "OOoMetaDataParser.h"
#import "OOoContentDataParser.h"
#import "OOoManifestParser.h"

/* a dictionary to hold the UTIs */
static NSDictionary *uti2kind;

typedef struct {
    unsigned short min_version;
    unsigned short general_flag;
    unsigned short compression;
    unsigned short lastmod_time;
    unsigned short lastmod_date;
    unsigned crc32;
    unsigned compressed_size;
    unsigned uncompressed_size;
    unsigned short filename_size;
    unsigned short extra_field_size;
    NSString *filename;
    NSString *extra_field;
} LocalFileHeader;

typedef struct {
    unsigned short creator_version;
    unsigned short min_version;
    unsigned short general_flag;
    unsigned short compression;
    unsigned short lastmod_time;
    unsigned short lastmod_date;
    unsigned crc32;
    unsigned compressed_size;
    unsigned uncompressed_size;
    unsigned short filename_size;
    unsigned short extra_field_size;
    unsigned short file_comment_size;
    unsigned short disk_num;
    unsigned short internal_attr;
    unsigned external_attr;
    unsigned offset;
    NSString *filename;
    NSString *extra_field;
    NSString *file_comment;
} CentralDirectoryEntry;

typedef struct {
    unsigned short disk_num;
    unsigned short cdir_disk;
    unsigned short disk_entries;
    unsigned short cdir_entries;
    unsigned cdir_size;
    unsigned cdir_offset;
    unsigned short comment_size;
    NSString *comment;
} CentralDirectoryEnd;

#define CDIR_ENTRY_SIG (0x02014b50)
#define LOC_FILE_HEADER_SIG (0x04034b50)
#define CDIR_END_SIG (0x06054b50)

static unsigned char readByte(NSFileHandle *file)
{
    if (file  == nil)
        return 0;
    NSData* tmpBuf = [file readDataOfLength: 1];
    if (tmpBuf == nil)
        return 0;
    unsigned char *d = (unsigned char*)[tmpBuf bytes];
    if (d == nil)
        return 0;
    return *d;
}

static unsigned short readShort(NSFileHandle *file)
{
    unsigned short p0 = (unsigned short)readByte(file);
    unsigned short p1 = (unsigned short)readByte(file);
    return (unsigned short)(p0|(p1<<8));
}

static unsigned readInt(NSFileHandle *file)
{
    unsigned p0 = (unsigned)readByte(file);
    unsigned p1 = (unsigned)readByte(file);
    unsigned p2 = (unsigned)readByte(file);
    unsigned p3 = (unsigned)readByte(file);
    return (unsigned)(p0|(p1<<8)|(p2<<16)|(p3<<24));
}

static bool readCentralDirectoryEnd(NSFileHandle *file, CentralDirectoryEnd *end)
{
    unsigned signature = readInt(file);
    if (signature != CDIR_END_SIG)
        return false;

    end->disk_num = readShort(file);
    end->cdir_disk = readShort(file);
    end->disk_entries = readShort(file);
    end->cdir_entries = readShort(file);
    end->cdir_size = readInt(file);
    end->cdir_offset = readInt(file);
    end->comment_size = readShort(file);
    NSData *data = [file readDataOfLength: end->comment_size];
    end->comment = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
    return true;
}

static bool readCentralDirectoryEntry(NSFileHandle *file, CentralDirectoryEntry *entry)
{
    unsigned signature = readInt(file);
    if (signature != CDIR_ENTRY_SIG)
        return false;

    entry->creator_version = readShort(file);
    entry->min_version = readShort(file);
    entry->general_flag = readShort(file);
    entry->compression = readShort(file);
    entry->lastmod_time = readShort(file);
    entry->lastmod_date = readShort(file);
    entry->crc32 = readInt(file);
    entry->compressed_size = readInt(file);
    entry->uncompressed_size = readInt(file);
    entry->filename_size = readShort(file);
    entry->extra_field_size = readShort(file);
    entry->file_comment_size = readShort(file);
    entry->disk_num = readShort(file);
    entry->internal_attr = readShort(file);
    entry->external_attr = readInt(file);
    entry->offset = readInt(file);
    NSData *data = [file readDataOfLength: entry->filename_size];
    entry->filename = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
    data = [file readDataOfLength: entry->extra_field_size];
    entry->extra_field = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
    data = [file readDataOfLength: entry->file_comment_size];
    entry->file_comment = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
    return true;
}

static bool readLocalFileHeader(NSFileHandle *file, LocalFileHeader *header)
{
    unsigned signature = readInt(file);
    if (signature != LOC_FILE_HEADER_SIG)
        return false;

    header->min_version = readShort(file);
    header->general_flag = readShort(file);
    header->compression = readShort(file);
    header->lastmod_time = readShort(file);
    header->lastmod_date = readShort(file);
    header->crc32 = readInt(file);
    header->compressed_size = readInt(file);
    header->uncompressed_size = readInt(file);
    header->filename_size = readShort(file);
    header->extra_field_size = readShort(file);
    NSData *data = [file readDataOfLength: header->filename_size];
    header->filename = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
    data = [file readDataOfLength: header->extra_field_size];
    header->extra_field = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
    return true;
}

static bool areHeadersConsistent(const LocalFileHeader *header, const CentralDirectoryEntry *entry)
{
    if (header->min_version != entry->min_version)
        return false;
    if (header->general_flag != entry->general_flag)
        return false;
    if (header->compression != entry->compression)
        return false;
    if (!(header->general_flag & 0x08))
    {
        if (header->crc32 != entry->crc32)
            return false;
        if (header->compressed_size != entry->compressed_size)
            return false;
        if (header->uncompressed_size != entry->uncompressed_size)
            return false;
    }
    return true;
}

static bool findCentralDirectoryEnd(NSFileHandle *file)
{
    // Assume the cdir end is in the last 1024 bytes
    // Scan backward from end of file for the end signature

    [file seekToEndOfFile];
    unsigned long long fileLength = [file offsetInFile];

    if (fileLength < 10)
        return false;

    [file seekToFileOffset: (fileLength - 4)];

    unsigned long long limit;
    if (fileLength > 1024)
        limit = fileLength - 1024;
    else
        limit = 0;

    unsigned long long offset;
    while ((offset = [file offsetInFile]) > limit)
    {
        unsigned signature = readInt(file);
        if (signature == CDIR_END_SIG)
        {
            // Seek back over the CDIR_END_SIG
            [file seekToFileOffset: offset];
            return true;
        }
        else
        {
            // Seek one byte back
            [file seekToFileOffset: (offset - 1)];
        }
    }
    return false;
}

static bool isZipFile(NSFileHandle *file)
{
    if (!findCentralDirectoryEnd(file))
        return false;
    CentralDirectoryEnd end;
    if (!readCentralDirectoryEnd(file, &end))
        return false;
    [file seekToFileOffset: end.cdir_offset];
    CentralDirectoryEntry entry;
    if (!readCentralDirectoryEntry(file, &entry))
        return false;
    [file seekToFileOffset: entry.offset];
    LocalFileHeader header;
    if (!readLocalFileHeader(file, &header))
        return false;
    if (!areHeadersConsistent(&header, &entry))
        return false;
    return true;
}

static bool findDataStream(NSFileHandle *file, CentralDirectoryEntry *entry, NSString *name)
{
    [file seekToEndOfFile];
    unsigned long long fileLength = [file offsetInFile];
    if (!findCentralDirectoryEnd(file))
        return false;
    CentralDirectoryEnd end;
    if (!readCentralDirectoryEnd(file, &end))
        return false;
    [file seekToFileOffset: end.cdir_offset];
    do
    {
        if (!readCentralDirectoryEntry(file, entry))
            return false;
        if ([entry->filename compare: name] == NSOrderedSame)
            break;
    }
    while ( [file offsetInFile] < fileLength && [file offsetInFile] < end.cdir_offset + end.cdir_size);
    if ([entry->filename compare: name] != NSOrderedSame)
        return false;
    [file seekToFileOffset: entry->offset];
    LocalFileHeader header;
    if (!readLocalFileHeader(file, &header))
        return false;
    if (!areHeadersConsistent(&header, entry))
        return false;
    return true;
}

static NSData *getUncompressedData(NSFileHandle *file, NSString *name)
{
    CentralDirectoryEntry entry;
    if (!findDataStream(file, &entry, name))
        return nil;
    if (!entry.compression)
        return [file readDataOfLength: entry.compressed_size];
    else
    {
        int ret;
        z_stream strm;

        /* allocate inflate state */
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.avail_in = 0;
        strm.next_in = Z_NULL;
        ret = inflateInit2(&strm,-MAX_WBITS);
        if (ret != Z_OK)
            return nil;

        NSData *compressedData = [file readDataOfLength: entry.compressed_size];

        strm.avail_in = [compressedData length];
        strm.next_in = (Bytef *)[compressedData bytes];

        Bytef *uncompressedData = (Bytef *)malloc(entry.uncompressed_size);
        if (!uncompressedData)
        {
            (void)inflateEnd(&strm);
            return nil;
        }
        strm.avail_out = entry.uncompressed_size;
        strm.next_out = uncompressedData;
        ret = inflate(&strm, Z_FINISH);
        switch (ret)
        {
        case Z_NEED_DICT:
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
            (void)inflateEnd(&strm);
            free(uncompressedData);
            return nil;
        }
        (void)inflateEnd(&strm);
        NSData *returnBuffer = [NSData dataWithBytes:(const void *)uncompressedData length:entry.uncompressed_size];
        free(uncompressedData);
        return returnBuffer;
    }
}

@implementation OOoSpotlightAndQuickLookImporter

/* initialize is only called once the first time this class is loaded */
+ (void)initialize
{
    static BOOL isInitialized = NO;
    if (isInitialized == NO) {
        NSMutableDictionary *temp = [NSMutableDictionary new];
        [temp setObject:@"OpenOffice.org 1.0 Text" forKey:@"org.openoffice.text"];
        [temp setObject:@"OpenDocument Text" forKey:@"org.oasis.opendocument.text"];
        [temp setObject:@"OpenOffice.org 1.0 Spreadsheet" forKey:@"org.openoffice.spreadsheet"];
        [temp setObject:@"OpenDocument Spreadsheet" forKey:@"org.oasis.opendocument.spreadsheet"];
        [temp setObject:@"OpenOffice.org 1.0 Presentation" forKey:@"org.openoffice.presentation"];
        [temp setObject:@"OpenDocument Presentation" forKey:@"org.oasis.opendocument.presentation"];
        [temp setObject:@"OpenOffice.org 1.0 Drawing" forKey:@"org.openoffice.graphics"];
        [temp setObject:@"OpenDocument Drawing" forKey:@"org.oasis.opendocument.graphics"];
        [temp setObject:@"OpenOffice.org 1.0 Master" forKey:@"org.openoffice.text-master"];
        [temp setObject:@"OpenDocument Master" forKey:@"org.oasis.opendocument.text-master"];
        [temp setObject:@"OpenOffice.org 1.0 Formula" forKey:@"org.openoffice.formula"];
        [temp setObject:@"OpenDocument Formula" forKey:@"org.oasis.opendocument.formula"];
        [temp setObject:@"OpenOffice.org 1.0 Text Template" forKey:@"org.openoffice.text-template"];
        [temp setObject:@"OpenDocument Text Template" forKey:@"org.oasis.opendocument.text-template"];
        [temp setObject:@"OpenOffice.org 1.0 Spreadsheet Template" forKey:@"org.openoffice.spreadsheet-template"];
        [temp setObject:@"OpenDocument Spreadsheet Template" forKey:@"org.oasis.opendocument.spreadsheet-template"];
        [temp setObject:@"OpenOffice.org 1.0 Presentation Template" forKey:@"org.openoffice.presentation-template"];
        [temp setObject:@"OpenDocument Presentation Template" forKey:@"org.oasis.opendocument.presentation-template"];
        [temp setObject:@"OpenOffice.org 1.0 Drawing Template" forKey:@"org.openoffice.graphics-template"];
        [temp setObject:@"OpenDocument Drawing Template" forKey:@"org.oasis.opendocument.graphics-template"];
        [temp setObject:@"OpenOffice.org 1.0 Database" forKey:@"org.openoffice.database"];
        [temp setObject:@"OpenDocument Chart" forKey:@"org.oasis.opendocument.chart"];

        uti2kind = [[NSDictionary dictionaryWithDictionary:temp] retain];
        [temp release];

        isInitialized = YES;
    }
}

/* importDocument is the real starting point for our plugin */
- (BOOL)importDocument:(NSString*)pathToFile contentType:(NSString*)contentTypeUTI attributes:(NSMutableDictionary*)attributes
{
    //NSLog(contentTypeUTI);
    //NSLog(pathToFile);

    NSString *itemKind = [uti2kind objectForKey:contentTypeUTI];
    if (itemKind != nil) {
        [attributes setObject:itemKind forKey:(NSString*)kMDItemKind];
    }

    //first check to see if this is a valid zipped file that contains a file "meta.xml"
    NSFileHandle *unzipFile = [self openZipFileAtPath:pathToFile];


    if (unzipFile == nil) {
        //NSLog(@"zip file not open");
        return NO;
    }

    //first get the metadata
    NSData *metaData = [self metaDataFileFromZip:unzipFile];
    if (metaData == nil) {
        [unzipFile closeFile];
        return YES;
    }

    [metaData retain];

    OOoMetaDataParser *parser = [OOoMetaDataParser new];
    if (parser != nil) {
        //parse and extract the data
        [parser parseXML:metaData intoDictionary:attributes];
    }

    [metaData release];
    [parser release];

    //and now get the content
    NSData *contentData = [self contentDataFileFromZip:unzipFile];
    if (contentData == nil) {
        [unzipFile closeFile];
        return YES;
    }

    [contentData retain];

    OOoContentDataParser *parser2 = [OOoContentDataParser new];
    if (parser2 != nil) {
        //parse and extract the data
        [parser2 parseXML:contentData intoDictionary:attributes];
    }

    [contentData release];
    [parser2 release];

    [unzipFile closeFile];

    return YES;
}

- (NSImage*)importDocumentThumbnail:(NSString*)pathToFile {
    if (!pathToFile || ![pathToFile length])
        return nil;

    NSFileHandle *unzipFile = [self openZipFileAtPath:pathToFile];
    if (!unzipFile)
        return nil;

    NSData *manifest = [self manifestFileFromZip:unzipFile];
    if (!manifest) {
        [unzipFile closeFile];
        return nil;
    }

    [manifest retain];

    NSMutableDictionary *attributes = [NSMutableDictionary new];

    OOoManifestParser *parser = [OOoManifestParser new];
    [parser parseXML:manifest intoDictionary:attributes];
    [parser release];

    NSMutableArray<NSString *> *pathArray = [NSMutableArray arrayWithCapacity:[attributes count]];

    // NeoOffice files have a low resolution .pdf thumbnail so if there is a .pdf
    // thumbnail available, use that
    NSString *path = [attributes objectForKey:MEDIA_TYPE_PDF];
    if (path && [path length])
        [pathArray addObject:path];

    // OpenOffice, NeoOffice, and LibreOffice files normally have a .png thumbnail
    path = [attributes objectForKey:MEDIA_TYPE_PNG];
    if (path && [path length])
        [pathArray addObject:path];

    for (NSString *key in [attributes allKeys]) {
        if (![key length] || [key isEqualToString:MEDIA_TYPE_PDF] || [key isEqualToString:MEDIA_TYPE_PNG])
            continue;
        path = [attributes objectForKey:key];
        if (path && [path length])
            [pathArray addObject:path];
    }

    [attributes release];
    [manifest release];

    NSImage *image = nil;
    for (path in pathArray) {
        NSData *imageData = getUncompressedData(unzipFile, path);
        if (imageData) {
            image = [[NSImage alloc] initWithData:imageData];
            if (image) {
                [image autorelease];
                break;
            }
        }
    }

    [unzipFile closeFile];

    return image;
}

/* openZipFileAtPath returns the file as a valid data structure or nil otherwise*/
- (NSFileHandle*)openZipFileAtPath:(NSString*)pathToFile
{
    NSFileHandle* unzipFile = nil;

    if ([pathToFile length] != 0)
    {
        unzipFile = [NSFileHandle fileHandleForReadingAtPath: pathToFile];
    }

    if (unzipFile == nil)
    {
        //NSLog(@"Cannot open %s",zipfilename);
        return nil;
    }

    if (!isZipFile(unzipFile))
    {
        [unzipFile closeFile];
        return nil;
    }
    //NSLog(@"%s opened",zipfilename);

    return unzipFile;
}

/* metaDataFileFromZip extracts the file meta.xml from the zip file and returns it as an NSData* structure
   or nil if the metadata is not present */
- (NSData*) metaDataFileFromZip:(NSFileHandle*)unzipFile
{
    if (unzipFile == nil)
        return nil;
    return getUncompressedData(unzipFile, @"meta.xml");
}

/* contentDataFileFromZip extracts the file content.xml from the zip file and returns it as an NSData* structure
   or nil if the metadata is not present */
- (NSData*) contentDataFileFromZip:(NSFileHandle*)unzipFile
{
    if (unzipFile == nil)
        return nil;
    return getUncompressedData(unzipFile, @"content.xml");
}

- (NSData*) manifestFileFromZip:(NSFileHandle*)unzipFile
{
    if (unzipFile == nil)
        return nil;
    return getUncompressedData(unzipFile, @"META-INF/manifest.xml");
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
