/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _LNKBASE_HXX
#define _LNKBASE_HXX

#ifndef _SOT_EXCHANGE_HXX //autogen
#include <sot/exchange.hxx>
#endif
#ifndef _TOOLS_REF_HXX
#include <tools/ref.hxx>
#endif
#ifndef _LINKSOURCE_HXX
#include <bf_so3/linksrc.hxx>
#endif

#ifndef INCLUDED_SO3DLLAPI_H
#include "bf_so3/so3dllapi.h"
#endif

namespace com { namespace sun { namespace star { namespace uno
{
    class Any;
    //class Type;
}}}}

namespace binfilter
{

struct ImplBaseLinkData;
class SvLinkManager;
class SvLinkSource;

#ifndef OBJECT_DDE_EXTERN
#define	OBJECT_INTERN		0x00
//#define	OBJECT_SO_EXTERN	0x01
#define	OBJECT_DDE_EXTERN	0x02
#endif

#define	OBJECT_CLIENT_SO			0x80 // ein Link
#define	OBJECT_CLIENT_DDE			0x81
//#define	OBJECT_CLIENT_OLE			0x82 // ein Ole-Link
//#define	OBJECT_CLIENT_OLE_CACHE  	0x83 // ein Ole-Link mit SvEmbeddedObject
#define	OBJECT_CLIENT_FILE			0x90
#define	OBJECT_CLIENT_GRF			0x91

enum so3link {
    // Ole2 compatibel und persistent
    LINKUPDATE_ALWAYS = 1,
    LINKUPDATE_ONCALL = 3,

    LINKUPDATE_END		// dummy!
};

class SO3_DLLPUBLIC SvBaseLink : public SvRefBase
{
    friend class SvLinkManager;
    friend class SvLinkSource;
private:
    SvLinkSourceRef			xObj;
    String					aLinkName;
    SvLinkManager* 			pLinkMgr;
    USHORT 					nObjType;
    BOOL					bVisible : 1;
    BOOL					bSynchron : 1;
    BOOL					bUseCache : 1;		// fuer GrafikLinks!

protected:
    void			SetObjType( USHORT );

                    // setzen des LinkSourceName ohne aktion
    SO3_DLLPRIVATE void			SetName( const String & rLn );

    ImplBaseLinkData* pImplData;

                    SvBaseLink( USHORT nLinkType, ULONG nContentType = FORMAT_STRING );
    virtual 		~SvBaseLink();

    void 			_GetRealObject( BOOL bConnect = TRUE );

    SO3_DLLPRIVATE SvLinkSource*	GetRealObject()
                    {
                        if( !xObj.Is() )
                            _GetRealObject();
                        return xObj;
                    }

public:
                    TYPEINFO();
                    // ask JP
    virtual void    Closed();

    USHORT			GetObjType() const { return nObjType; }

    SvLinkSource*	GetObj() const	{ return xObj; }

    void    		SetLinkSourceName( const String & rName );
    String		 	GetLinkSourceName() const;

    virtual void 	DataChanged( const String & rMimeType,
                                const ::com::sun::star::uno::Any & rValue );

    void			SetUpdateMode( USHORT );
    USHORT 			GetUpdateMode() const;
    ULONG  			GetContentType() const;
    BOOL 			SetContentType( ULONG nType );

    SvLinkManager*	GetLinkManager()			{ return pLinkMgr; }
    const SvLinkManager* GetLinkManager() const	{ return pLinkMgr; }

    BOOL			Update();
    void			Disconnect();

    virtual BOOL 	Edit( Window* );

        // soll der Link im Dialog angezeigt werden ? (Links im Link im ...)
    BOOL 	        IsVisible() const   		{ return bVisible; }
    void 	        SetVisible( BOOL bFlag )	{ bVisible = bFlag; }
        // soll der Link synchron oder asynchron geladen werden?
    BOOL 	        IsSynchron() const   		{ return bSynchron; }
    void 	        SetSynchron( BOOL bFlag )	{ bSynchron = bFlag; }

    BOOL 	        IsUseCache() const   		{ return bUseCache; }
    void 			SetUseCache( BOOL bFlag )	{ bUseCache = bFlag; }
};

SV_DECL_IMPL_REF(SvBaseLink);

}

#endif
