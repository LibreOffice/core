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



#ifndef _SAMPLE_COLLELEM_HXX
#define _SAMPLE_COLLELEM_HXX

#include <basic/sbxobj.hxx>

// Das Sample-Element ist ein kleines Objekt, das die Properties
// Name und Value enthÑlt sowie die Methode Say, die den Åbergebenen
// Text mit dem eigenen Namen verkoppelt. Der Name ist von aussen setzbar.
// Die Implementation arbeitet ausschliesslich mit dynamischen Elementen.

class SampleElement : public SbxObject
{
    // Broadcaster Notification
    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType );
public:
    SampleElement( const String& );
};

#endif
