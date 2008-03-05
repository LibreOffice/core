/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: embeddedwavaudiofile.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:40:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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

#ifndef OOX_DRAWINGML_EMBEDDEDWAVAUDIOFILE_HXX
#define OOX_DRAWINGML_EMBEDDEDWAVAUDIOFILE_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/xml/sax/XFastAttributeList.hpp>

#include "oox/core/fragmenthandler.hxx"

namespace oox { namespace drawingml {

    struct EmbeddedWAVAudioFile
    {
        EmbeddedWAVAudioFile()
            : mbBuiltIn(false)
            {
            }
        bool mbBuiltIn;
        ::rtl::OUString msName;
        ::rtl::OUString msLink;
    };

    void getEmbeddedWAVAudioFile(
            const ::oox::core::Relations& rRelations,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs,
            EmbeddedWAVAudioFile & aAudio );

} }


#endif
