/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <rtl/ustring.hxx>

#include <premac.h>
#import <Foundation/Foundation.h>
#include <postmac.h>

#import "util_cocoa.hxx"

using namespace rtl;

bool JvmfwkUtil_isLoadableJVM( OUString const & aURL )
{
    bool bRet = false;

    if ( aURL.getLength() )
    {
        NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

        NSString *pString = [NSString stringWithCharacters:reinterpret_cast<unichar const *>(aURL.getStr()) length:aURL.getLength()];
        if ( pString )
        {
            NSURL *pURL = nil;
            NSURL *pTmpURL = [NSURL URLWithString:pString];
            if ( pTmpURL )
            {
              #if MACOSX_SDK_VERSION >= 1060
                pTmpURL = [pTmpURL filePathURL];
              #else
                pTmpURL = [ NSURL URLWithString:(NSString*)( CFURLCopyFileSystemPath( (CFURLRef)pTmpURL, kCFURLPOSIXPathStyle ) ) ];
              #endif
            }
            if ( pTmpURL )
            {
              #if MACOSX_SDK_VERSION < 1060
                pTmpURL = [ NSURL URLWithString:[[pTmpURL path] stringByStandardizingPath] ];
              #else
                pTmpURL = [pTmpURL URLByStandardizingPath];
              #endif
            }
            if ( pTmpURL )
            {
              #if MACOSX_SDK_VERSION < 1060
                pTmpURL = [ NSURL URLWithString:[[pTmpURL path] stringByResolvingSymlinksInPath] ];
              #else
                pTmpURL = [pTmpURL URLByResolvingSymlinksInPath];
              #endif
            }
            if ( pTmpURL )
            {
              #if MACOSX_SDK_VERSION >= 1080
                NSURL *pJVMsDirURL = [NSURL URLWithString:@"file:///Library/Java/JavaVirtualMachines/"];
              #else
                NSURL *pJVMsDirURL = [NSURL URLWithString:@"file:///System/Library/Frameworks/JavaVM.framework/Versions/"];
              #endif
                if ( pJVMsDirURL )
                {
                  #if MACOSX_SDK_VERSION >= 1060
                    pJVMsDirURL = [pJVMsDirURL filePathURL];
                  #else
                    pJVMsDirURL = [ NSURL URLWithString:(NSString*)( CFURLCopyFileSystemPath( (CFURLRef)pJVMsDirURL, kCFURLPOSIXPathStyle ) ) ];
                  #endif
                }
                if ( pJVMsDirURL )
                {
                  #if MACOSX_SDK_VERSION < 1060
                    pJVMsDirURL = [ NSURL URLWithString:[[pJVMsDirURL path] stringByStandardizingPath] ];
                  #else
                    pJVMsDirURL = [pJVMsDirURL URLByStandardizingPath];
                  #endif
                }
                // The JVM directory must not contain softlinks or the JavaVM
                // framework bug will occur so don't resolve softlinks in the
                // JVM directory
                if ( pJVMsDirURL )
                {
                    NSString *pTmpURLString = [pTmpURL absoluteString];
                    NSString *pJVMsDirURLString = [pJVMsDirURL absoluteString];
                    if ( pTmpURLString && pJVMsDirURLString && [pJVMsDirURLString length] )
                    {
                        NSRange aJVMsDirURLRange = [pTmpURLString rangeOfString:pJVMsDirURLString];
                        if ( !aJVMsDirURLRange.location && aJVMsDirURLRange.length )
                            pURL = pTmpURL;
                    }
                }
            }

#if MACOSX_SDK_VERSION >= 1080
            while ( pURL )
            {
                // Check if this is a valid bundle
                NSURL *pContentsURL = nil;
                pContentsURL = [pURL URLByAppendingPathComponent:@"Contents"]; //[NSURL URLWithString:[[pURL path] stringByAppendingPathComponent:@"Contents"]];
                BOOL isDir = NO;
                if ( pContentsURL && [[NSFileManager defaultManager] fileExistsAtPath:[pContentsURL path] isDirectory:&isDir] && isDir )
                {
                    NSBundle *pBundle = [NSBundle bundleWithURL:pURL]; //[NSBundle bundleWithPath:[pURL path]];
                    if ( pBundle )
                    {
                        // Make sure that this bundle's Info.plist has the
                        // proper JVM keys to supports loading via JNI. If
                        // this bundle is a valid JVM and these keys
                        // are missing, loading the JVM will cause OS X's
                        // JavaVM framework to display a dialog and invoke
                        // exit() when loaded via JNI on OS X 10.10.
                        NSDictionary *pInfo = [pBundle infoDictionary];
                        if ( pInfo )
                        {
                            NSDictionary *pJavaVM = [pInfo objectForKey:@"JavaVM"];
                            if ( pJavaVM && [pJavaVM isKindOfClass:[NSDictionary class]] )
                            {
                                NSArray *pJVMCapabilities = [pJavaVM objectForKey:@"JVMCapabilities"];
                                if ( pJVMCapabilities )
                                {
                                    if ( [pJVMCapabilities indexOfObjectIdenticalTo:@"JNI"] == NSNotFound )
                                    {
                                        if ( [pJVMCapabilities isKindOfClass:[NSMutableArray class]] )
                                        {
                                            [(NSMutableArray *)pJVMCapabilities addObject:@"JNI"];
                                            bRet = true;
                                        }
                                        else if ( [pJavaVM isKindOfClass:[NSMutableDictionary class]] )
                                        {
                                            NSMutableArray *pNewJVMCapabilities = [NSMutableArray arrayWithCapacity:[pJVMCapabilities count] + 1];
                                            if ( pNewJVMCapabilities )
                                            {
                                                [pNewJVMCapabilities addObject:@"JNI"];
                                                [(NSMutableDictionary *)pJavaVM setObject:pNewJVMCapabilities forKey:@"JVMCapabilities"];
                                                bRet = true;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        bRet = true;
                                    }
                                }
                            }
                        }
                    }
                }

                NSURL *pOldURL = pURL;
                pURL = [pURL URLByDeletingLastPathComponent]; //[NSURL URLWithString:[[pURL path] stringByDeletingLastPathComponent]];
                if ( pURL )
                {
                    pURL = [pURL URLByStandardizingPath]; //[NSURL URLWithString:[[pURL path] stringByStandardizingPath]];
                    if ( pURL )
                    {
                        pURL = [pURL URLByResolvingSymlinksInPath]; //[NSURL URLWithString:[[pURL path] stringByResolvingSymlinksInPath]];
                        if ( pURL && [pURL isEqual:pOldURL] )
                            pURL = nil;
                    }
                }
            }
#else // i.e. MACOSX_SDK_VERSION < 1080
            if ( pURL )
                bRet = true;
#endif
        }

        [pPool release];
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
