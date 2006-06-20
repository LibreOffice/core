/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ContentInfo.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 06:14:03 $
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
#ifndef _CONTENT_INFO_HXX_
#define _CONTENT_INFO_HXX_

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEl_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _ZIP_PACKAGE_FOLDER_HXX
#include <ZipPackageFolder.hxx>
#endif
#ifndef _ZIP_PACKAGE_STREAM_HXX
#include <ZipPackageStream.hxx>
#endif

namespace com { namespace sun { namespace star { namespace packages {
class ContentInfo : public cppu::OWeakObject
{
public:
    com::sun::star::uno::Reference < com::sun::star::lang::XUnoTunnel > xTunnel;
    bool bFolder;
    union
    {
        ZipPackageFolder *pFolder;
        ZipPackageStream *pStream;
    };
    ContentInfo ( ZipPackageStream * pNewStream )
    : xTunnel ( pNewStream )
    , bFolder ( false )
    , pStream ( pNewStream )
    {
    }
    ContentInfo ( ZipPackageFolder * pNewFolder )
    : xTunnel ( pNewFolder )
    , bFolder ( true )
    , pFolder ( pNewFolder )
    {
    }
    virtual ~ContentInfo ()
    {
        if ( bFolder )
            pFolder->releaseUpwardRef();
        else
            pStream->clearParent();
    }
};
} } } }
#endif
