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
            // Ignore all but Oracle's JDK as loading Apple's Java and Oracle's
            // JRE will cause macOS's JavaVM framework to display a dialog and
            // invoke exit() when loaded via JNI on macOS 10.10
            NSURL *pURL = [NSURL URLWithString:pString];
            if ( pURL )
                pURL = [pURL filePathURL];
            if ( pURL )
                pURL = [pURL URLByStandardizingPath];
            if ( pURL )
                pURL = [pURL URLByResolvingSymlinksInPath];

            while ( pURL )
            {
                // Check if this is a valid bundle
                NSNumber *pDir = nil;
                NSURL *pContentsURL = [pURL URLByAppendingPathComponent:@"Contents"];
                if ( pContentsURL && [pContentsURL getResourceValue:&pDir forKey:NSURLIsDirectoryKey error:nil] && pDir && [pDir boolValue] )
                {
                    NSBundle *pBundle = [NSBundle bundleWithURL:pURL];
                    if ( pBundle )
                    {
                        // Make sure that this bundle's Info.plist has the
                        // proper JVM keys to supports loading via JNI. If
                        // this bundle is a valid JVM and these keys
                        // are missing, loading the JVM will cause macOS's
                        // JavaVM framework to display a dialog and invoke
                        // exit() when loaded via JNI on macOS 10.10.
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
                                            [static_cast<NSMutableArray *>(pJVMCapabilities) addObject:@"JNI"];
                                            bRet = true;
                                        }
                                        else if ( [pJavaVM isKindOfClass:[NSMutableDictionary class]] )
                                        {
                                            NSMutableArray *pNewJVMCapabilities = [NSMutableArray arrayWithCapacity:[pJVMCapabilities count] + 1];
                                            if ( pNewJVMCapabilities )
                                            {
                                                [pNewJVMCapabilities addObject:@"JNI"];
                                                [static_cast<NSMutableDictionary *>(pJavaVM) setObject:pNewJVMCapabilities forKey:@"JVMCapabilities"];
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
                pURL = [pURL URLByDeletingLastPathComponent];
                if ( pURL )
                {
                    pURL = [pURL URLByStandardizingPath];
                    if ( pURL )
                    {
                        pURL = [pURL URLByResolvingSymlinksInPath];
                        if ( pURL && [pURL isEqual:pOldURL] )
                            pURL = nil;
                    }
                }
            }
        }

        [pPool release];
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
