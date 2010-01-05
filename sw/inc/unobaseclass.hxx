/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unobaseclass.hxx,v $
 * $Revision: 1.3 $
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
#ifndef _UNOBASECLASS_HXX
#define _UNOBASECLASS_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XEnumeration.hpp>

#include <cppuhelper/implbase2.hxx>


class SfxPoolItem;
class SwClient;
class SwDoc;


typedef ::cppu::WeakImplHelper2
<   ::com::sun::star::lang::XServiceInfo
,   ::com::sun::star::container::XEnumeration
>
SwSimpleEnumeration_Base;


/* -----------------29.04.98 07:35-------------------
 *
 * --------------------------------------------------*/
enum CursorType
{
    CURSOR_INVALID,
    CURSOR_BODY,
    CURSOR_FRAME,
    CURSOR_TBLTEXT,
    CURSOR_FOOTNOTE,
    CURSOR_HEADER,
    CURSOR_FOOTER,
    CURSOR_REDLINE,
    CURSOR_ALL,         // for Search&Replace
    CURSOR_SELECTION,   // create a paragraph enumeration from
                        // a text range or cursor
    CURSOR_SELECTION_IN_TABLE,
    CURSOR_META,         // meta/meta-field
};

/*-----------------04.03.98 11:54-------------------
    Start/EndAction or Start/EndAllAction
  -------------------------------------------------- */
class UnoActionContext
{
    private:
        SwDoc * m_pDoc;

    public:
        UnoActionContext(SwDoc *const pDoc);
        ~UnoActionContext();

        void InvalidateDocument() { m_pDoc = 0; }
};

/* -----------------07.07.98 12:03-------------------
    interrupt Actions for a little while
   -------------------------------------------------- */
class UnoActionRemoveContext
{
    private:
        SwDoc *const m_pDoc;

    public:
        UnoActionRemoveContext(SwDoc *const pDoc);
        ~UnoActionRemoveContext();
};


::com::sun::star::uno::Sequence< sal_Int8 > CreateUnoTunnelId();

/// helper function for implementing SwClient::Modify
void ClientModify(SwClient* pClient, SfxPoolItem *pOld, SfxPoolItem *pNew);

#endif

