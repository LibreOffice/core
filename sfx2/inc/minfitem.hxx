/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: minfitem.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:03:56 $
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
#ifndef _SFX_MINFITEM_HXX
#define _SFX_MINFITEM_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif
class BasicManager;

class SFX2_DLLPUBLIC SfxMacroInfoItem: public SfxPoolItem
{
    const BasicManager*     pBasicManager;
    String                  aLibName;
    String                  aModuleName;
    String                  aMethodName;
    String                  aCommentText;

public:
    TYPEINFO();
    SfxMacroInfoItem( USHORT nWhich,
                    const BasicManager* pMgr,
                    const String &rLibName,
                    const String &rModuleName,
                    const String &rMethodName,
                    const String &rComment);

    SfxMacroInfoItem( const SfxMacroInfoItem& );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
    String                  GetComment() const
                                { return aCommentText; }
    void                    SetComment( const String& r )
                                { aCommentText = r; }
    String                  GetMethod() const
                                { return aMethodName; }
    void                    SetMethod( const String& r )
                                { aMethodName = r; }
    String                  GetModule() const
                                { return aModuleName; }
    void                    SetModule( const String& r )
                                { aModuleName = r; }
    String                  GetLib() const
                                { return aLibName; }
    void                    SetLib( const String& r )
                                { aLibName = r; }
    const BasicManager*     GetBasicManager() const
                            { return pBasicManager; }
    String                  GetQualifiedName() const;
};

#endif
