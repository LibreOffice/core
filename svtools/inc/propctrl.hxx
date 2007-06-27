/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propctrl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 21:05:18 $
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

/*
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#include <vcl/sound.hxx>
#include "sbx.hxx"
#include "sbxbase.hxx"
#include "sbxres.hxx"

#ifndef _SFXBRDCST_HXX //autogen
#include <svtools/brdcst.hxx>
#endif
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
    BOOL bSimpleHistory;

    // Methode zum Anlegen/Aktivieren der Controller
    void showObject( const UsrAny& aToShowObj );
    String getPath( void );

public:
    // Provisorischer Ctor mit Parent-Window
    SimplePropertyEditor_Impl( Window *pParent );
    ~SimplePropertyEditor_Impl();

    // HACK fuer History-Test
    void enableSimpleHistory( BOOL bHistory_ ) { bSimpleHistory = bHistory_; }

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




