/*************************************************************************
 *
 *  $RCSfile: propctrl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:58 $
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


#ifndef _USR_INTROSP_HXX
#include <usr/inspect.hxx>
#endif
#ifndef _USR_SERINFO_HXX
#include <usr/serinfo.hxx>
#endif
#ifndef _USR_INTROSP_HXX
#include <usr/introsp.hxx>
#endif

#include <propctrl.hxx>
#include <property.hxx>


// Controller-Implementation
class PropertyEditorControler_Impl : public SvPropertyDataControl
{
    XIntrospectionAccessRef mxUnoAccess;
    PropertySequence mPropSeq;
    XPropertyEditorRef mxEditor;
    SvPropertyBox* mpPropBox;
    UsrAny maUnoObj;

public:
    // Provisorisch direkt Window mitgeben
    PropertyEditorControler_Impl( SvPropertyBox* pPropBox_ );
    //SimplePropertyEditor_Impl( void );

    // Objekt zum Editieren setzen, dies loest das Eintragen
    // der Properties in die PropertyBox aus
    void setObject( XPropertyEditorRef xEditor_, const UsrAny& aToEditObj,
        /* HACK fuer History-Interface*/String aPath, BOOL bBack=FALSE, BOOL bForward=FALSE );

    /* SPAETER
    SMART_UNO_DECLARATION(ImplIntrospection,UsrObject);

    // Methoden von XInterface
    XInterface *    queryInterface( Uik aUik );
    XIdlClassRef    getIdlClass();
    */

    // Methoden von SvPropertyDataControl
    virtual void                    Modified(   const String& aName,
                                                const String& aVal,
                                                void* pData);

    virtual void                    Clicked(    const String& aName,
                                                const String& aVal,
                                                void* pData);

    virtual void                    Commit(     const String& aName,
                                                const String& aVal,
                                                void* pData);

    virtual void                    Select(     const String& aName,
                                                void* pData);

    virtual void                    LinkClicked(const String& aName,
                                                void* pData);

    // TODO: Das muss raus, sehr unglueckliche Schnittstelle
    // PropertyBox erzwingt Zustand des Controllers
    virtual String                  GetTheCorrectProperty() const;
};

// Methoden von XPropertyEditor
PropertyEditorControler_Impl::PropertyEditorControler_Impl( SvPropertyBox* pPropBox_ )
{
    mpPropBox = pPropBox_;
}

void PropertyEditorControler_Impl::setObject( XPropertyEditorRef xEditor_, const UsrAny& aToEditObj,
    /* HACK fuer History-Interface*/ String aPath, BOOL bBack, BOOL bForward )
{
    static XIntrospectionRef xIntrospection;

    // Ohne Fenster laeuft gar nix
    if( !mpPropBox )
        return;

    // Fenster aufraeumen
    mpPropBox->ClearAll();

    // Editor und Objekt übernehmen
    mxEditor = xEditor_;
    maUnoObj = aToEditObj;

    if( !xIntrospection.is() )
    {
        // Introspection-Service holen
        XServiceManagerRef xServiceManager = getGlobalServiceManager();
        XServiceProviderRef xProv = xServiceManager->getServiceProvider
            ( "com.sun.star.beans.Introspection", UikSequence(), UikSequence() );
        xIntrospection = (XIntrospection *)xProv->newInstance()
            ->queryInterface( XIntrospection::getSmartUik() );
    }
    if( !xIntrospection.is() )
        return;

    // und unspecten
    mxUnoAccess = xIntrospection->inspect( maUnoObj );
    if( !mxUnoAccess.Is() )
        return;

    // Uns als Controler anmelden
    mpPropBox->SetController( this );

    // Properties anlegen
    mPropSeq = mxUnoAccess->getProperties();
    UINT32 nPropCount = mPropSeq.getLen();
    const Property* pProps = mPropSeq.getConstArray();

    // 1. Seite anlegen
    USHORT nPropPageId = mpPropBox->AppendPage("Properties");

    // Beim Eintragen solls nicht flimmern
    mpPropBox->DisableUpdate();

    // Dummy-Properties fuer Path und Navigation
    SvPropertyData aProperty;
    if( aPath.Len() )
    {
        // Interface und Structs werden Hyperlinks
        aProperty.bIsHyperLink = FALSE;
        aProperty.bIsLocked = TRUE;
        aProperty.bHasVisibleXButton = FALSE;
        aProperty.eKind = KOC_EDIT;
        aProperty.pControl = NULL;
        aProperty.pDataPtr = NULL;
        aProperty.aName = "Path";
        aProperty.aValue = aPath;
        mpPropBox->InsertEntry( aProperty );
    }
    if( bBack || bForward )
    {
        // Interface und Structs werden Hyperlinks
        aProperty.bIsHyperLink = TRUE;
        aProperty.bIsLocked = TRUE;
        // HACK, solange Hyperlink nicht funktioniert
        aProperty.bHasVisibleXButton = aProperty.bIsHyperLink;
        aProperty.eKind = KOC_EDIT;
        UINT32 iHandle;
        aProperty.pControl = NULL;
        if( bBack )
        {
            iHandle = 1000001;
            aProperty.pDataPtr = (void*)iHandle;
            aProperty.aName = "<-";
            aProperty.aValue = "Back";
            mpPropBox->InsertEntry( aProperty );
        }
        if( bForward )
        {
            iHandle = 1000000;
            aProperty.pDataPtr = (void*)iHandle;
            aProperty.aName = "->";
            aProperty.aValue = "Forward";
            mpPropBox->InsertEntry( aProperty );
        }
    }

    // Properties eintragen
    // TODO: Wo kommen die Strings her
    UINT32 i;
    for( i = 0 ; i < nPropCount ; i++ )
    {
        const Property& rProp = pProps[ i ];

        // TypeClass des Property ermitteln
        XIdlClassRef xPropClass = rProp.Type;
        if( !xPropClass.is() )
        {
            DBG_ERROR( "PropertyEditorControler_Impl::Commit(), Property without type" )
            return;
        }
        TypeClass eType = xPropClass->getTypeClass();

        // Interface und Structs werden Hyperlinks
        aProperty.bIsHyperLink = ( eType == TYPECLASS_INTERFACE || eType == TYPECLASS_STRUCT );
        aProperty.bIsLocked = ((rProp.Attributes & PROPERTY_READONLY) != 0 );

        // HACK, solange Hyperlink nicht funktioniert
        aProperty.bHasVisibleXButton = aProperty.bIsHyperLink;

        // Wert holen und in String wandeln
        UsrAny aVal = mxUnoAccess->getPropertyValueByIndex( maUnoObj, i );
        String aStrVal = AnyToString( aVal );

        // Properties reinbraten
        aProperty.eKind = KOC_EDIT;
        aProperty.aName = rProp.Name;
        aProperty.aValue = aStrVal;
        aProperty.pDataPtr = (void*)i;
        aProperty.pControl = NULL;
        //aProperty.theValues.Insert(new String("1"),aProperty.theValues.Count());
        //aProperty.theValues.Insert(new String("2"),aProperty.theValues.Count());
        //aProperty.theValues.Insert(new String("3"),aProperty.theValues.Count());
        //aProperty.theValues.Insert(new String("4"),aProperty.theValues.Count());
        mpPropBox->InsertEntry( aProperty );
    }

    // 2. Seite fuer Listener
    // TODO: Wo kommen die Eintraege her
    USHORT nListenerPageId = mpPropBox->AppendPage("Listener");

    XIdlClassSequence aSupportedListenerSeq = mxUnoAccess->getSupportedListeners();
    const XIdlClassRef* pListenerArray = aSupportedListenerSeq.getConstArray();
    UINT32 nIfaceCount = aSupportedListenerSeq.getLen();

    // Property-Data vorfuellen
    aProperty.eKind = KOC_EDIT;
    //aProperty.eKind = KOC_UNDEFINED;
    aProperty.aValue = "Listener-Value";
    aProperty.bHasVisibleXButton = TRUE;
    // TEST
    //aProperty.bIsHyperLink = TRUE;
    aProperty.bIsHyperLink = FALSE;
    aProperty.bIsLocked = TRUE;
    //aProperty.bIsLocked = FALSE;
    aProperty.pDataPtr = NULL;
    aProperty.pControl = NULL;

    for( UINT32 j = 0 ; j < nIfaceCount ; j++ )
    {
        const XIdlClassRef& rxIfaceClass = pListenerArray[j];
        aProperty.aName = rxIfaceClass->getName();
        mpPropBox->InsertEntry( aProperty );
    }
    mpPropBox->EnableUpdate();
    mpPropBox->SetPage( nPropPageId );
}

void PropertyEditorControler_Impl::Modified
( const String& aName, const String& aVal, void* pData)
{
}

void PropertyEditorControler_Impl::Clicked
( const String& aName, const String& aVal, void* pData)
{
    // HACK, solange LinkClicked nicht funktioniert
    UINT32 iPos = (UINT32)pData;
    UINT32 nPropCount = mPropSeq.getLen();
    if( iPos >= nPropCount )
    {
        // Spezial-IDs fuer forward/back?
        BOOL bForward = (iPos == 1000000);
        BOOL bBack = (iPos == 1000001);
        if( bForward || bBack )
        {
            // Unterstuetzt der PropertyEditor das?
            XPropertyEditorNavigationRef xPropEdNav = (XPropertyEditorNavigation*)
                mxEditor->queryInterface( XPropertyEditorNavigation::getSmartUik() );
            if( xPropEdNav.is() )
            {
                if( bForward )
                    xPropEdNav->forward();
                else
                    xPropEdNav->back();
            }
        }
        return;
    }

    const Property* pProps = mPropSeq.getConstArray();
    const Property& rProp = pProps[ iPos ];
    XIdlClassRef xPropClass = rProp.Type;
    if( !xPropClass.is() )
    {
        DBG_ERROR( "PropertyEditorControler_Impl::Commit(), Property without type" )
        return;
    }
    TypeClass eType = xPropClass->getTypeClass();
    if( eType == TYPECLASS_INTERFACE || eType == TYPECLASS_STRUCT )
        LinkClicked( aName, pData );
}

void PropertyEditorControler_Impl::Commit
( const String& aName, const String& aVal, void* pData)
{
    UINT32 iPos = (UINT32)pData;
    UINT32 nPropCount = mPropSeq.getLen();
    if( iPos >= nPropCount )
        return;

    // String in Property-Typ wandeln
    const Property* pProps = mPropSeq.getConstArray();
    const Property& rProp = pProps[ iPos ];
    XIdlClassRef xPropClass = rProp.Type;
    if( !xPropClass.is() )
    {
        DBG_ERROR( "PropertyEditorControler_Impl::Commit(), Property without type" )
        return;
    }
    TypeClass eType = xPropClass->getTypeClass();
    UsrAny aValue = StringToAny( aVal, eType );

    // Wert setzen
    mxUnoAccess->setPropertyValueByIndex( maUnoObj, iPos, aValue );

    // Wert neu holen und ggf. neu setzen
    UsrAny aNewVal = mxUnoAccess->getPropertyValueByIndex( maUnoObj, iPos );
    String aNewStrVal = AnyToString( aNewVal );
    if( aNewStrVal != aVal )
        mpPropBox->SetPropertyValue( aName, aNewStrVal );
}

void PropertyEditorControler_Impl::Select
( const String& aName, void* pData)
{
}

void PropertyEditorControler_Impl::LinkClicked(const String& aName, void* pData)
{
    UINT32 iPos = (UINT32)pData;
    UINT32 nPropCount = mPropSeq.getLen();
    if( iPos >= nPropCount )
        return;

    // Wert holen und an Master-Controller zurueckgeben
    UsrAny aNewObj = mxUnoAccess->getPropertyValueByIndex( maUnoObj, iPos );
    mxEditor->setObject( aNewObj, aName );
}


// TODO: Das muss raus, sehr unglueckliche Schnittstelle
// PropertyBox erzwingt Zustand des Controllers
String PropertyEditorControler_Impl::GetTheCorrectProperty() const
{
    return String();
}


SMART_UNO_IMPLEMENTATION(SimplePropertyEditor_Impl,UsrObject);

// Methoden von XInterface
XInterface * SimplePropertyEditor_Impl::queryInterface( Uik aUik )
{
    if( aUik == XPropertyEditor::getSmartUik() )
        return (XPropertyEditor *)this;
    if( aUik == XPropertyEditorNavigation::getSmartUik() )
        return (XPropertyEditorNavigation *)this;
    return UsrObject::queryInterface( aUik );
}

XIdlClassRef SimplePropertyEditor_Impl::getIdlClass()
{
    // TODO: Unterstuetzen
    return NULL;
}


// Methoden von SimplePropertyEditor_Impl
SimplePropertyEditor_Impl::SimplePropertyEditor_Impl( Window *pParent )
    : maHistorySeq( 10 ), maHistoryNames( 10 ), bSimpleHistory( FALSE )
{
    //XVCLComponent xC = pParent->getVCLComponent
    //xC->addVCLComponentListener( MyListener )

    pActiveControler = NULL;
    mpPropBox = new SvPropertyBox( pParent );
    mpPropBox->Show();

    long cxOut = pParent->GetOutputSizePixel().Width();
    long cyOut = pParent->GetOutputSizePixel().Height();
    Size aSize( cxOut, cyOut );
    mpPropBox->SetPosSizePixel( Point( 0, 0 ), aSize );

    mnHistoryCount = 0;
    mnActualHistoryLevel = -1;
}

SimplePropertyEditor_Impl::~SimplePropertyEditor_Impl()
{
    delete mpPropBox;
    if( pActiveControler )
        delete pActiveControler;
}

// Private Methode zum Anlegen/Aktivieren der Controller
void SimplePropertyEditor_Impl::showObject( const UsrAny& aToShowObj )
{
    if( pActiveControler )
        delete pActiveControler;

    // Neuen Controller auf der Wiese anlegen (TODO: Controller cachen?)
    pActiveControler = new PropertyEditorControler_Impl( mpPropBox );

    XPropertyEditorRef xThis = (XPropertyEditor *)this;
    pActiveControler->setObject( xThis, aToShowObj,
        /*aPath*/bSimpleHistory ? getPath() : String(),
        /*bBack*/bSimpleHistory && mnActualHistoryLevel > 0,
        /*bForward*/bSimpleHistory && (INT32)mnHistoryCount > mnActualHistoryLevel );
}

String SimplePropertyEditor_Impl::getPath( void )
{
    String aRetStr;
    const String* pStr = maHistoryNames.getConstArray();
    for( INT32 i = 0 ; i <= mnActualHistoryLevel ; i++ )
    {
        String aName = pStr[i];

        // Root speziell behandeln
        if( i == 0 )
        {
            aRetStr += aName;
        }
        else
        {
            // Ist es ein Index?
            long l = (long)aName;
            String aNumStr( l );
            if( aNumStr == aName )
            {
                aRetStr += '[';
                aRetStr += aName;
                aRetStr += ']';
            }
            else
            {
                aRetStr += '.';
                aRetStr += aName;
            }
        }
    }
    return aRetStr;
}

// Methoden von XPropertyEditor
void SimplePropertyEditor_Impl::setObject( const UsrAny& aToEditObj, const XubString& aObjName )
{
    // History pflegen
    mnActualHistoryLevel++;
    mnHistoryCount = (UINT32)mnActualHistoryLevel;
    UINT32 iHistorySize = maHistorySeq.getLen();
    if( mnHistoryCount > iHistorySize )
    {
        maHistorySeq.realloc( iHistorySize + 10 );
        maHistoryNames.realloc( iHistorySize + 10 );
    }

    // Neues Object eintragen
    maHistorySeq.getArray()[ mnHistoryCount ] = aToEditObj;
    maHistoryNames.getArray()[ mnHistoryCount ] = aObjName;

    // Object anzeigen
    showObject( aToEditObj );
}

// Methoden von PropertyEditorNavigation
void SimplePropertyEditor_Impl::forward(void)
{
    if( (INT32)mnHistoryCount > mnActualHistoryLevel )
    {
        // Naechstes Object darstellen
        mnActualHistoryLevel++;
        showObject( maHistorySeq.getConstArray()[mnActualHistoryLevel] );
    }
}

void SimplePropertyEditor_Impl::back(void)
{
    if( mnActualHistoryLevel > 0 )
    {
        // Voriges Object darstellen
        mnActualHistoryLevel--;
        showObject( maHistorySeq.getConstArray()[mnActualHistoryLevel] );
    }
}


