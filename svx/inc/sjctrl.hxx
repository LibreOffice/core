/*************************************************************************
 *
 *  $RCSfile: sjctrl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:00:59 $
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

#ifdef SOLAR_JAVA

#ifndef _SFXSFXJS_HXX
#include <sfx2/sfxjs.hxx>
#endif

class VCControl;
class VCForm;
class VCManager;
class VCRadioManager;


// Basis-Klasse aller VCControl-Wrapper-Klassen
// Haelt Verweis auf das Control und handelt einige gemeinsame Properties
class ImpSjJScriptControlBase
{
  protected:
    VCControl* pControl;

  public:
    // Beibehalten bis zum Vollupdate
      /*
    String      getName() const;
    void        setName( const String & aStrg );
    String      getValue() const;
    String      getType() const;
    */

    // Methoden von Java --> C++ (f"ur Properties)
    //--------------------------------------------
    String      ImpGetName() const;
    void        ImpSetName( const String & aStrg );
    String      ImpGetValue() const;
    void        ImpSetValue( const String & aStrg );
    String      ImpGetType() const;

    // Konstruktor
    ImpSjJScriptControlBase::ImpSjJScriptControlBase( VCControl* _pControl );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Button-Control
class ImpSjJScriptButtonObject : public SjJScriptButtonObject, ImpSjJScriptControlBase
{
  public:
    // Methoden von Java --> C++ (f"ur Properties)
    //--------------------------------------------
    // Basisklassen-Properties, werden an ImpSjJScriptControlBase durchgereicht
    virtual String      getName() const;
    virtual void        setName( const String & aStrg );
    virtual String      getValue() const;
    virtual void        setValue( const String & aStrg );
    virtual String      getType() const;

    // Methoden des Objektes
    virtual void        click();

    // Events (C++ --> Java)
    //----------------------
#if SUPD > 356 || defined(AB_EVENT_TEST)
    // Neue Event-Schnittstelle mit Function-Objekten
    virtual SjJScriptFunctionObject* getOnClick_Fct() const;
    virtual void setOnClick_Fct( SjJScriptFunctionObject* pFunctionObject );
#endif
    virtual String      getOnClick() const;
    virtual void        setOnClick( const String & aSourceStrg );
            void        onClick();

    // Konstruktor
    ImpSjJScriptButtonObject( VCControl* _pControl, SjJSbxObject * p );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// PasswordControl (Edit-Control zur Password-Eingabe)
class ImpSjJScriptPasswordObject : public SjJScriptPasswordObject, ImpSjJScriptControlBase
{
  public:
    // Methoden von Java --> C++ (f"ur Properties)
    //--------------------------------------------
    // Basisklassen-Properties, werden an ImpSjJScriptControlBase durchgereicht
    virtual String      getName() const;
    virtual void        setName( const String & aStrg );
    virtual String      getValue() const;
    virtual String      getType() const;

    virtual String      getDefaultValue() const;
    virtual void        setDefaultValue( const String & aStrg );
    virtual void        setValue( const String & aStrg );

    // Methoden des Objektes
    virtual void        focus();
    virtual void        blur();
    virtual void        select();

    // Events (C++ --> Java)
    //----------------------
    // keine !

    // Konstruktor
    ImpSjJScriptPasswordObject( VCControl* _pControl, SjJSbxObject * p );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class ImpSjJScriptCheckboxObject : public SjJScriptCheckboxObject, ImpSjJScriptControlBase
{
  public:
    // Methoden von Java --> C++ (f"ur Properties)
    //--------------------------------------------
    // Basisklassen-Properties, werden an ImpSjJScriptControlBase durchgereicht
    virtual String      getName() const;
    virtual void        setName( const String & aStrg );
    virtual String      getValue() const;
    virtual String      getType() const;

    virtual BOOL        getChecked() const;
    virtual void        setChecked( BOOL bCheck );
    virtual BOOL        getDefaultChecked() const;
    virtual void        setDefaultChecked( BOOL bCheck );
    // getValue() wird in der Basisklasse definiert
    virtual void        setValue( const String & aStrg );

    // Methoden des Objektes
    virtual void        click();

    // Events (C++ --> Java)
    //----------------------
#if SUPD > 356 || defined(AB_EVENT_TEST)
    // Neue Event-Schnittstelle mit Function-Objekten
    virtual SjJScriptFunctionObject* getOnClick_Fct() const;
    virtual void setOnClick_Fct( SjJScriptFunctionObject* pFunctionObject );
#endif
    virtual String      getOnClick() const;
    virtual void        setOnClick( const String & aSourceStrg );
            void        onClick();

    // Konstruktor
    ImpSjJScriptCheckboxObject( VCControl* _pControl, SjJSbxObject * p );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Wrapper-Klasse fuer RadioArray
class ImpSjJScriptRadioArray: public SjJSbxArrayObject
{
    VCRadioManager* pMgr;
public:
    INT32 ImpSjJScriptRadioArray::getLength() const;
    SjJSbxObject* ImpSjJScriptRadioArray::getElement( INT32 nIndex ) const;
    SjJSbxObject* ImpSjJScriptRadioArray::getElement_String( const String & aName ) const;

    // Konstruktor
    ImpSjJScriptRadioArray::ImpSjJScriptRadioArray( VCRadioManager* _pMgr, SjJSbxObject * p );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class ImpSjJScriptRadioObject : public SjJScriptRadioObject, ImpSjJScriptControlBase
{
  public:
    // Methoden von Java --> C++ (f"ur Properties)
    //--------------------------------------------
    // Basisklassen-Properties, werden an ImpSjJScriptControlBase durchgereicht
    virtual String      getName() const;
    virtual void        setName( const String & aStrg );
    virtual String      getValue() const;
    virtual String      getType() const;

    // Properties von SjJScriptCheckboxObject
    virtual BOOL        getChecked() const;
    virtual void        setChecked( BOOL bCheck );
    virtual BOOL        getDefaultChecked() const;
    virtual void        setDefaultChecked( BOOL bCheck );
    // getValue() wird in der Basisklasse definiert
    virtual void        setValue( const String & aStrg );

    // yyy Raus zum inkompatiblen
    //virtual INT32     getLength();

    // Methoden des Objektes
    virtual void        click();

    // Events (C++ --> Java)
#if SUPD > 356 || defined(AB_EVENT_TEST)
    // Neue Event-Schnittstelle mit Function-Objekten
    virtual SjJScriptFunctionObject* getOnClick_Fct() const;
    virtual void setOnClick_Fct( SjJScriptFunctionObject* pFunctionObject );
#endif
    virtual String      getOnClick() const;
    virtual void        setOnClick( const String & aSourceStrg );
            void        onClick();

    // Konstruktor
    ImpSjJScriptRadioObject( VCControl* _pControl, SjJSbxObject * p );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class ImpSjJScriptSelectObject : public SjJScriptSelectObject, ImpSjJScriptControlBase
{
  public:
    // Methoden von Java --> C++ (f"ur Properties)
    //--------------------------------------------
    // Basisklassen-Properties, werden an ImpSjJScriptControlBase durchgereicht
    virtual String      getName() const;
    virtual void        setName( const String & aStrg );
    virtual String      getValue() const;
    virtual String      getType() const;

    virtual INT32       getLength();
    virtual INT32       getSelectedIndex() const;
    virtual void        setSelectedIndex( INT32 nNo );

    // liefert eine Array von Option-Objekten
    //yyy Raus zum inkompatiblen
    //virtual SjJSbxArrayObject * getOptions();

    // Methoden des Objektes
    virtual void        focus();
    virtual void        blur();

    // Events (C++ --> Java)
    //----------------------
#if SUPD > 356 || defined(AB_EVENT_TEST)
    // Neue Event-Schnittstelle mit Function-Objekten
    virtual SjJScriptFunctionObject* getOnBlur_Fct() const;
    virtual void setOnBlur_Fct( SjJScriptFunctionObject* pFunctionObject );
    virtual SjJScriptFunctionObject* getOnChange_Fct() const;
    virtual void setOnChange_Fct( SjJScriptFunctionObject* pFunctionObject );
    virtual SjJScriptFunctionObject* getOnFocus_Fct() const;
    virtual void setOnFocus_Fct( SjJScriptFunctionObject* pFunctionObject );
#endif
    virtual String      getOnBlur() const;
    virtual void        setOnBlur( const String & aSourceStrg );
            void        onBlur();
    virtual String      getOnChange() const;
    virtual void        setOnChange( const String & aSourceStrg );
            void        onChange();
    virtual String      getOnFocus() const;
    virtual void        setOnFocus( const String & aSourceStrg );
            void        onFocus();

    // SjJSbxArrayObject-Methoden
    virtual INT32           getLength() const;
    // Zugriff "uber den Index
    virtual SjJSbxObject *  getElement( INT32 nIndex ) const;
    // Zugriff "uber den Namen
    // native Java-Methoden k"onnen (noch) nicht "uberladen werden
    //virtual SjJSbxObject *    getElement_String( const String & aName ) const;

    // Konstruktor
    ImpSjJScriptSelectObject( VCControl* _pControl, SjJSbxObject * p );

    // zum Setzen von Eintr"agen am Objket (zum Aufruf von Java aus) 15.1.1997
    virtual void        setOption( INT32 nIndex, const String & aOptionText,
                                   const String & aOptionValue,
                                   BOOL bDefaultSelected, BOOL bSelected );

    // Schnittstelle fuer die Properties der Option-Objekte
    virtual BOOL        getOptionDefaultSelected( INT32 nIndex ) const;
    virtual void        setOptionDefaultSelected( BOOL bSelected, INT32 nIndex );
    virtual BOOL        getOptionSelected( INT32 nIndex ) const;
    virtual void        setOptionSelected( BOOL bSelected, INT32 nIndex );
    virtual String      getOptionText( INT32 nIndex ) const;
    virtual void        setOptionText( const String & sText, INT32 nIndex );
    virtual void        setOptionValue( const String & sText, INT32 nIndex );
    virtual String      getOptionValue( INT32 nIndex ) const;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Wieso von Controlbase abgeleitet?

class ImpSjJScriptOptionObject : public SjJScriptOptionObject, ImpSjJScriptControlBase
{
    USHORT              nListBoxIndex;
  public:
    // Methoden von Java --> C++ (f"ur Properties)
    //--------------------------------------------
    virtual BOOL        getdefaultSelected() const;
    virtual void        setdefaultSelected( BOOL bSelected );
    virtual INT32       getIndex() const;
    virtual BOOL        getSelected() const;
    virtual void        setSelected( BOOL bSelected );


    //yyy Zum Vollupdate raus:
    /*
    virtual INT32       getSelectedIndex() const;
    virtual void        setSelectedIndex( INT32 nNo );
    */


    virtual String      getText() const;
    virtual void        setText( const String & sText );
    // getValue() wird in der Basisklasse definiert
    virtual String      getValue() const;
    virtual void        setValue( const String & aStrg );


    // DUMMY-Methoden
    virtual String getName() const { return "";}
    virtual void setName( const String &s) { }
    virtual String getType() const { return "";}
    // Methoden des Objektes
    // keine !

    // Events (C++ --> Java)
    //----------------------
    // keine !

    // Konstruktor
    ImpSjJScriptOptionObject( VCControl* _pControl, SjJSbxObject * p, USHORT _nListBoxIndex );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Eigene Klasse fuer forms-Array anlegen
class SjJSbxFormsArray: public SfxJSArray
{
    VCManager* pManager;

  public:
    // SjJSbxArrayObject-Methoden
    virtual INT32           getLength() const;
    // Zugriff "uber den Index
    virtual SjJSbxObject *  getElement( INT32 nIndex ) const;
    // Zugriff "uber den Namen
    // native Java-Methoden k"onnen (noch) nicht "uberladen werden
    virtual SjJSbxObject *  getElement_String( const String & aName ) const;

    // Konstruktor
    SjJSbxFormsArray( VCManager * _pManager, SjJSbxObject * p );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class ImpSjJScriptTextObject : public SjJScriptTextObject, ImpSjJScriptControlBase
{
  public:
    // Methoden von Java --> C++ (f"ur Properties)
    //--------------------------------------------
    // Basisklassen-Properties, werden an ImpSjJScriptControlBase durchgereicht
    virtual String      getName() const;
    virtual void        setName( const String & aStrg );
    virtual String      getValue() const;
    virtual String      getType() const;

    virtual String      getDefaultValue() const;
    virtual void        setDefaultValue( const String & aStrg );
    // getValue() wird in der Basisklasse definiert
    virtual void        setValue( const String & aStrg );

    // Methoden des Objektes
    virtual void        focus();
    virtual void        blur();
    virtual void        select();

    // Events (C++ --> Java)
    //----------------------
#if SUPD > 356 || defined(AB_EVENT_TEST)
    // Neue Event-Schnittstelle mit Function-Objekten
    virtual SjJScriptFunctionObject* getOnBlur_Fct() const;
    virtual void setOnBlur_Fct( SjJScriptFunctionObject* pFunctionObject );
    virtual SjJScriptFunctionObject* getOnChange_Fct() const;
    virtual void setOnChange_Fct( SjJScriptFunctionObject* pFunctionObject );
    virtual SjJScriptFunctionObject* getOnFocus_Fct() const;
    virtual void setOnFocus_Fct( SjJScriptFunctionObject* pFunctionObject );
    virtual SjJScriptFunctionObject* getOnSelect_Fct() const;
    virtual void setOnSelect_Fct( SjJScriptFunctionObject* pFunctionObject );
#endif
    virtual String      getOnBlur() const;
    virtual void        setOnBlur( const String & aSourceStrg );
            void        onBlur();
    virtual String      getOnChange() const;
    virtual void        setOnChange( const String & aSourceStrg );
            void        onChange();
    virtual String      getOnFocus() const;
    virtual void        setOnFocus( const String & aSourceStrg );
            void        onFocus();
    virtual String      getOnSelect() const;
    virtual void        setOnSelect( const String & aSourceStrg );
            void        onSelect();

    // Konstruktor
    ImpSjJScriptTextObject( VCControl* _pControl, SjJSbxObject * p );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class ImpSjJScriptTextareaObject : public SjJScriptTextareaObject, ImpSjJScriptControlBase
{
  public:
    // Methoden von Java --> C++ (f"ur Properties)
    //--------------------------------------------
    // Basisklassen-Properties, werden an ImpSjJScriptControlBase durchgereicht
    virtual String      getName() const;
    virtual void        setName( const String & aStrg );
    virtual String      getValue() const;
    virtual String      getType() const;

    virtual String      getDefaultValue() const;
    virtual void        setDefaultValue( const String & aStrg );
    // getValue() wird in der Basisklasse definiert
    virtual void        setValue( const String & aStrg );

    // Methoden des Objektes
    virtual void        focus();
    virtual void        blur();
    virtual void        select();

    // Events (C++ --> Java)
    //----------------------
#if SUPD > 356 || defined(AB_EVENT_TEST)
    // Neue Event-Schnittstelle mit Function-Objekten
    virtual SjJScriptFunctionObject* getOnBlur_Fct() const;
    virtual void setOnBlur_Fct( SjJScriptFunctionObject* pFunctionObject );
    virtual SjJScriptFunctionObject* getOnChange_Fct() const;
    virtual void setOnChange_Fct( SjJScriptFunctionObject* pFunctionObject );
    virtual SjJScriptFunctionObject* getOnFocus_Fct() const;
    virtual void setOnFocus_Fct( SjJScriptFunctionObject* pFunctionObject );
    virtual SjJScriptFunctionObject* getOnSelect_Fct() const;
    virtual void setOnSelect_Fct( SjJScriptFunctionObject* pFunctionObject );
#endif
    virtual String      getOnBlur() const;
    virtual void        setOnBlur( const String & aSourceStrg );
            void        onBlur();
    virtual String      getOnChange() const;
    virtual void        setOnChange( const String & aSourceStrg );
            void        onChange();
    virtual String      getOnFocus() const;
    virtual void        setOnFocus( const String & aSourceStrg );
            void        onFocus();
    virtual String      getOnSelect() const;
    virtual void        setOnSelect( const String & aSourceStrg );
            void        onSelect();

    // Konstruktor
    ImpSjJScriptTextareaObject( VCControl* _pControl, SjJSbxObject * p );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class ImpSjJScriptSubmitObject : public SjJScriptSubmitObject, ImpSjJScriptControlBase
{
    // Identisch mit ButtonObject
  public:
    // Methoden von Java --> C++ (f"ur Properties)
    //--------------------------------------------
    // Basisklassen-Properties, werden an ImpSjJScriptControlBase durchgereicht
    virtual String      getName() const;
    virtual void        setName( const String & aStrg );
    virtual String      getValue() const;
    virtual void        setValue( const String & aStrg );
    virtual String      getType() const;

    // Methoden des Objektes
    virtual void        click();

    // Events (C++ --> Java)
    //----------------------
#if SUPD > 356 || defined(AB_EVENT_TEST)
    // Neue Event-Schnittstelle mit Function-Objekten
    virtual SjJScriptFunctionObject* getOnClick_Fct() const;
    virtual void setOnClick_Fct( SjJScriptFunctionObject* pFunctionObject );
#endif
    virtual String      getOnClick() const;
    virtual void        setOnClick( const String & aSourceStrg );
            void        onClick();

    // Konstruktor
    ImpSjJScriptSubmitObject( VCControl* _pControl, SjJSbxObject * p );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class ImpSjJScriptResetObject : public SjJScriptResetObject, ImpSjJScriptControlBase
{
    // Identisch mit ButtonObject
  public:
    // Methoden von Java --> C++ (f"ur Properties)
    //--------------------------------------------
    // Basisklassen-Properties, werden an ImpSjJScriptControlBase durchgereicht
    virtual String      getName() const;
    virtual void        setName( const String & aStrg );
    virtual String      getValue() const;
    virtual void        setValue( const String & aStrg );
    virtual String      getType() const;

    // Methoden des Objektes
    virtual void        click();

    // Events (C++ --> Java)
    //----------------------
#if SUPD > 356 || defined(AB_EVENT_TEST)
    // Neue Event-Schnittstelle mit Function-Objekten
    virtual SjJScriptFunctionObject* getOnClick_Fct() const;
    virtual void setOnClick_Fct( SjJScriptFunctionObject* pFunctionObject );
#endif
    virtual String      getOnClick() const;
    virtual void        setOnClick( const String & aSourceStrg );
            void        onClick();

    // Konstruktor
    ImpSjJScriptResetObject( VCControl* _pControl, SjJSbxObject * p );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class ImpSjJScriptHiddenObject : public SjJScriptHiddenObject, ImpSjJScriptControlBase
{
  public:
    // Methoden von Java --> C++ (f"ur Properties)
    //--------------------------------------------
    // Basisklassen-Properties, werden an ImpSjJScriptControlBase durchgereicht
    virtual String      getName() const;
    virtual void        setName( const String & aStrg );
    virtual String      getValue() const;
    virtual String      getType() const;

    // getValue() wird in der Basisklasse definiert
    virtual void        setValue( const String & aStrg );

    // Methoden des Objektes
    // keine !

    // Events (C++ --> Java)
    //----------------------
    // keine !

    // Konstruktor
    ImpSjJScriptHiddenObject( VCControl* _pControl, SjJSbxObject * p );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class ImpSjJScriptFileUploadObject : public SjJScriptFileUploadObject, ImpSjJScriptControlBase
{
  public:
    // Methoden von Java --> C++ (f"ur Properties)
    //--------------------------------------------
    // Basisklassen-Properties, werden an ImpSjJScriptControlBase durchgereicht
    virtual String      getName() const;
    virtual void        setName( const String & aStrg );
    virtual String      getValue() const;
    virtual String      getType() const;

    // Methoden des Objektes
    // keine !

    // Events (C++ --> Java)
    //----------------------
    // keine !

    // Konstruktor
    ImpSjJScriptFileUploadObject( VCControl* _pControl, SjJSbxObject * p );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class ImpSjJScriptFormObject : public SjJScriptFormObject
{
    VCForm* pVCForm;

  public:
    // Methoden von Java --> C++ (f"ur Properties)
    //--------------------------------------------
    virtual String      getName() const;
    virtual void        setName( const String & aStrg );
    virtual String      getValue() const;
    virtual String      getType() const;

    virtual String      getAction() const;
    virtual void        setAction( const String & sValue );
    virtual String      getEncoding() const;
    virtual void        setEncoding( const String & sValue );
    virtual String      getMethod() const;
    virtual void        setMethod( const String & sValue );
    virtual String      getTarget() const;
    virtual void        setTarget( const String & sValue );

    // SjJSbxArrayObject-Methoden
    INT32               getLength() const;
    SjJSbxObject*       getElement( INT32 nIndex ) const;
    SjJSbxObject*       getElement_String( const String & aName ) const;

    // liefert ein Array-Objekt mit Elementen vom Typ JScriptAbstractControlsObject
    // yyy Raus zum inkompatiblen
    //virtual SjJSbxArrayObject * getElements();

    // Zugriff ueber den Namen eines Controls (falls ein Name zugeordnet ist)
    // yyy Raus zum inkompatiblen
    //virtual SjJSbxObject * getElement( const String & aName );

    // Methoden des Objektes
    virtual void        submit();
    virtual void        reset();

    // Events (C++ --> Java)
    //----------------------
#if SUPD > 356 || defined(AB_EVENT_TEST)
    // Neue Event-Schnittstelle mit Function-Objekten
    virtual SjJScriptFunctionObject* getOnSubmit_Fct() const;
    virtual void setOnSubmit_Fct( SjJScriptFunctionObject* pFunctionObject );
    virtual SjJScriptFunctionObject* getOnReset_Fct() const;
    virtual void setOnReset_Fct( SjJScriptFunctionObject* pFunctionObject );
#endif
    virtual String      getOnSubmit() const;
    virtual void        setOnSubmit( const String & aSourceStrg );
            void        onSubmit();
    virtual String      getOnReset() const;
    virtual void        setOnReset( const String & aSourceStrg );
            void        onReset();

    // Konstruktor
    ImpSjJScriptFormObject( VCForm* _pVCForm, SjJSbxObject * p );
};

#endif
