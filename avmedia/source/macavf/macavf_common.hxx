/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#ifndef MACAVF_COMMON_HXX
#define MACAVF_COMMON_HXX

#ifdef MACOSX
#include <premac.h>
#import <Cocoa/Cocoa.h>
#import <AVFoundation/AVFoundation.h>
#include <postmac.h>
#endif
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/factory.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/media/XManager.hpp>


#define AVMEDIA_MACAVF_MANAGER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Manager_MacAVF"
#define AVMEDIA_MACAVF_MANAGER_SERVICENAME "com.sun.star.media.Manager_MacAVF"

#define AVMEDIA_MACAVF_PLAYER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Player_MacAVF"
#define AVMEDIA_MACAVF_PLAYER_SERVICENAME "com.sun.star.media.Player_MacAVF"

#define AVMEDIA_MACAVF_WINDOW_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Window_MacAVF"
#define AVMEDIA_MACAVF_WINDOW_SERVICENAME "com.sun.star.media.Window_MacAVF"

#define AVMEDIA_MACAVF_FRAMEGRABBER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.FrameGrabber_MacAVF"
#define AVMEDIA_MACAVF_FRAMEGRABBER_SERVICENAME "com.sun.star.media.FrameGrabber_MacAVF"


// MacAVObserver handles the notifications used in the AVFoundation framework

@interface MacAVObserverObject : NSObject
- (void)observeValueForKeyPath:(NSString*)pKeyPath ofObject:(id)pObject change:(NSDictionary*)pChangeDict context:(void*)pContext;
- (void)onNotification:(NSNotification*)pNotification;
@end

namespace avmedia { namespace macavf {

class MacAVObserverHandler
{
private:
    static MacAVObserverObject* mpMacAVObserverObject;
public:
    MacAVObserverObject* getObserver( void ) const;
    virtual bool handleObservation( NSString* pKeyPath ) = 0;
};

}}

#endif // MACAVF_COMMON_HXX

