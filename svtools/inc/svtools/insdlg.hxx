/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: insdlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:24:10 $
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
#ifndef _SVTOOLS_INSDLG_HXX
#define _SVTOOLS_INSDLG_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDDEDOBJECT_HPP_
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif

#ifndef _GLOBNAME_HXX //autogen
#include <tools/globname.hxx>
#endif
#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif

#ifndef _OWNCONT_HXX
#include <svtools/ownlist.hxx>
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif

class SvObjectServer
{
private:
    SvGlobalName    aClassName;
    String          aHumanName;

public:
    SvObjectServer( const SvGlobalName & rClassP, const String & rHumanP ) :
        aClassName( rClassP ),
        aHumanName( rHumanP ) {}

    const SvGlobalName &    GetClassName() const { return aClassName; }
    const String &          GetHumanName() const { return aHumanName; }
};

class SVT_DLLPUBLIC SvObjectServerList
{
    PRV_SV_DECL_OWNER_LIST(SvObjectServerList,SvObjectServer)
    const SvObjectServer *  Get( const String & rHumanName ) const;
    const SvObjectServer *  Get( const SvGlobalName & ) const;
    void                    Remove( const SvGlobalName & );
    void                    FillInsertObjects();
};

class SVT_DLLPUBLIC SvPasteObjectHelper
{
public:
    static String GetSotFormatUIName( SotFormatStringId nId );
    static sal_Bool GetEmbeddedName(const TransferableDataHelper& rData, String& _rName, String& _rSource, SotFormatStringId& _nFormat);
};

#endif // _SVTOOLS_INSDLG_HXX

