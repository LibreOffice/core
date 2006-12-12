/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pptexsoundcollection.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:41:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SD_PPT_EXSOUNDCOLLECTION_HXX
#define _SD_PPT_EXSOUNDCOLLECTION_HXX

#ifdef DBG_ANIM_LOG
#include <stdio.h>
#endif
#include <tools/string.hxx>
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
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
