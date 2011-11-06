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



#ifndef _SD_PPT_EXSOUNDCOLLECTION_HXX
#define _SD_PPT_EXSOUNDCOLLECTION_HXX

#ifdef DBG_ANIM_LOG
#include <stdio.h>
#endif
#include <tools/string.hxx>
#include <tools/stream.hxx>
#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#include <list>

namespace ppt
{

class ExSoundEntry
{
        sal_uInt32              nFileSize;
        String                  aSoundURL;

        String                  ImplGetName() const;
        String                  ImplGetExtension() const;

    public :

        sal_Bool                IsSameURL( const String& rURL ) const;
        sal_uInt32              GetFileSize( ) const { return nFileSize; };

                                ExSoundEntry( const String& rSoundURL );

        // returns the size of a complete SoundContainer
        sal_uInt32              GetSize( sal_uInt32 nId ) const;
        void                    Write( SvStream& rSt, sal_uInt32 nId );
};

class ExSoundCollection : private List
{
        const ExSoundEntry*     ImplGetByIndex( sal_uInt32 nId ) const;

    public:

                                ExSoundCollection() {}
                                ~ExSoundCollection();

        sal_uInt32              GetId( const String& );

        // returns the size of a complete SoundCollectionContainer
        sal_uInt32              GetSize() const;
        void                    Write( SvStream& rSt );
};

} // namespace ppt

#endif
