#include <rtl/ustring.hxx>

#include <premac.h>
#import <Foundation/Foundation.h>
#include <postmac.h>

#import "util_cocoa.hxx"

using namespace rtl;

bool JvmfwkUtil_isLoadableJVM( OUString aURL )
{
    bool bRet = false;

    if ( aURL.getLength() )
    {
        NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

        NSString *pString = [NSString stringWithCharacters:aURL.getStr() length:aURL.getLength()];
        if ( pString )
        {
            NSURL *pURL = nil;

            // Ignore all but Oracle's JDK as loading Apple's Java and Oracle's
            // JRE will cause OS X's JavaVM framework to display a dialog and
            // invoke exit() when loaded via JNI on OS X 10.10
            NSURL *pTmpURL = [NSURL URLWithString:pString];
            if ( pTmpURL )
                pTmpURL = [pTmpURL filePathURL];
            if ( pTmpURL )
                pTmpURL = [pTmpURL URLByStandardizingPath];
            if ( pTmpURL )
                pTmpURL = [pTmpURL URLByResolvingSymlinksInPath];
            if ( pTmpURL )
            {
                NSURL *pJVMsDirURL = [NSURL URLWithString:@"file:///Library/Java/JavaVirtualMachines/"];
                if ( pJVMsDirURL )
                    pJVMsDirURL= [pJVMsDirURL filePathURL];
                if ( pJVMsDirURL )
                    pJVMsDirURL = [pJVMsDirURL URLByStandardizingPath];
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
