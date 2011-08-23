/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _LINKSRC_HXX
#define _LINKSRC_HXX

#include <tools/ref.hxx>
#include <tools/rtti.hxx>

#ifndef _SAL_CONFIG_H
#include <sal/config.h>
#define _SAL_CONFIG_H
#endif

#include "bf_so3/so3dllapi.h"

namespace com { namespace sun { namespace star { namespace uno
{
    class Any;
    //class Type;
}}}}
class Window;
class String;

#ifndef ADVISEMODE_NODATA
// Muessen die gleichen Werte wie Ole2 ADVF_* sein
#define ADVISEMODE_NODATA       0x01
//#define ADVISEMODE_PRIMEFIRST   0x02
#define ADVISEMODE_ONLYONCE     0x04
//#define ADVISEMODE_DATAONSTOP   0x40
#endif

namespace binfilter
{

class SvBaseLink;
struct SvLinkSource_Impl;

class SO3_DLLPUBLIC SvLinkSource : public SvRefBase
{
private:
    SvLinkSource_Impl *	pImpl; // compatible area
public:
                        TYPEINFO();

                        SvLinkSource();
    virtual				~SvLinkSource();

//	SvBaseLink*			GetDataBaseLink() const;
    BOOL				HasDataLinks( const SvBaseLink* = 0 ) const;

    void				Closed();

    void				SetUpdateTimeout( ULONG nTime );
                        // notify the sink, the mime type is not
                        // a selection criterion
    void				DataChanged( const String & rMimeType,
                                    const ::com::sun::star::uno::Any & rVal );
    void				SendDataChanged();
    void				NotifyDataChanged();

    virtual BOOL		Connect( SvBaseLink * );
    virtual BOOL		GetData( ::com::sun::star::uno::Any & rData /*out param*/,
                                const String & rMimeType,
                                BOOL bSynchron = FALSE );

                        // TRUE => waitinmg for data
    virtual BOOL		IsPending() const;
                        // TRUE => data complete loaded
    virtual BOOL		IsDataComplete() const;

    virtual String		Edit( Window *, SvBaseLink * );

    void				AddDataAdvise( SvBaseLink *, const String & rMimeType,
                                        USHORT nAdviceMode );
    void				RemoveAllDataAdvise( SvBaseLink * );

    void				AddConnectAdvise( SvBaseLink * );
    void				RemoveConnectAdvise( SvBaseLink * );
};

SV_DECL_IMPL_REF(SvLinkSource);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
