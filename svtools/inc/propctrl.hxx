/*************************************************************************
 *
 *  $RCSfile: propctrl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
#include <brdcst.hxx>
#endif
  */


#ifndef __PROPED_HXX__
#include <svtools/proped.hxx>
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




