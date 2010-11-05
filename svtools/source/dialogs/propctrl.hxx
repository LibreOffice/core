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

/*
#include <tools/stream.hxx>
#include <vcl/sound.hxx>
#include "sbx.hxx"
#include "sbxbase.hxx"
#include "sbxres.hxx"
#include <svl/brdcst.hxx>
  */


#ifndef __PROPED_HXX__
#include <proped.hxx>
#endif
#ifndef _UNO_HXX
#include <usr/uno.hxx>
#endif
#ifndef _USR_SEQU_HXX
#include <usr/sequ.hxx>
#endif
#ifndef __TOOLSIDL_HXX__
#include <usr/toolsidl.hxx>
#endif

/*
class XPropertyEditor
    : public XInterface
{
public:

    virtual void setObject(const UsrAny& aToInspectObj) = 0;

    static Uik getSmartUik() { return(385); }
};
*/

class PropertyEditorControler_Impl;
class SvPropertyBox;
class Window;

class SimplePropertyEditor_Impl :
    public XPropertyEditor,
    public XPropertyEditorNavigation,
    public UsrObject
{
    PropertyEditorControler_Impl* pActiveControler;
    SvPropertyBox* mpPropBox;
    UsrAny maStartUnoObj;
    UsrAny maActiveUnoObj;

    // History der Objekte speichern
    AnySequence maHistorySeq;
    WSStringSequence maHistoryNames;
    UINT32 mnHistoryCount;
    INT32 mnActualHistoryLevel;

    // Einfache History via Dummy-Properties
    sal_Bool bSimpleHistory;

    // Methode zum Anlegen/Aktivieren der Controller
    void showObject( const UsrAny& aToShowObj );
    String getPath( void );

public:
    // Provisorischer Ctor mit Parent-Window
    SimplePropertyEditor_Impl( Window *pParent );
    ~SimplePropertyEditor_Impl();

    // HACK fuer History-Test
    void enableSimpleHistory( sal_Bool bHistory_ ) { bSimpleHistory = bHistory_; }

    SMART_UNO_DECLARATION(ImplIntrospection,UsrObject);

    // Methoden von XInterface
    XInterface *    queryInterface( Uik aUik );
    XIdlClassRef    getIdlClass();

    // Methoden von XPropertyEditor
    virtual void setObject(const UsrAny& aToInspectObj, const XubString& aObjName);

    // Methoden von PropertyEditorNavigation
    virtual void forward(void);
    virtual void back(void);

};




