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

#ifdef SOLAR_JAVA

#include <sfx2/sfxjs.hxx>

class VCControl;
class VCForm;
class VCManager;
class VCRadioManager;

// Base class of all VCControl wrapper classes
// Keeps reference to the Control and handles some common properties
class ImpSjJScriptControlBase
{
  protected:
    VCControl* pControl;

  public:
    // Methods from Java --> C++ (for properties)
    //--------------------------------------------
    String      ImpGetName() const;
    void        ImpSetName( const String & aStrg );
    String      ImpGetValue() const;
    void        ImpSetValue( const String & aStrg );
    String      ImpGetType() const;

    // Constructor
    ImpSjJScriptControlBase::ImpSjJScriptControlBase( VCControl* _pControl );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Button-Control
class ImpSjJScriptButtonObject : public SjJScriptButtonObject, ImpSjJScriptControlBase
{
  public:
    // Methods from Java --> C++ (for properties)
    //--------------------------------------------
    // Base class properties, are passed through to ImpSjJScriptControlBase
    virtual String      getName() const;
    virtual void        setName( const String & aStrg );
    virtual String      getValue() const;
    virtual void        setValue( const String & aStrg );
    virtual String      getType() const;

    // Methods of the object
    virtual void        click();

    // Events (C++ --> Java)
    //----------------------
    // New event interface with function objects
    virtual SjJScriptFunctionObject* getOnClick_Fct() const;
    virtual void setOnClick_Fct( SjJScriptFunctionObject* pFunctionObject );
    virtual String      getOnClick() const;
    virtual void        setOnClick( const String & aSourceStrg );
            void        onClick();

    // constructor
    ImpSjJScriptButtonObject( VCControl* _pControl, SjJSbxObject * p );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// PasswordControl (edit control for entering password)
class ImpSjJScriptPasswordObject : public SjJScriptPasswordObject, ImpSjJScriptControlBase
{
  public:
    // Methods from Java --> C++ (for properties)
    //--------------------------------------------
    // Base class properties, are passed through to ImpSjJScriptControlBase
    virtual String      getName() const;
    virtual void        setName( const String & aStrg );
    virtual String      getValue() const;
    virtual String      getType() const;

    virtual String      getDefaultValue() const;
    virtual void        setDefaultValue( const String & aStrg );
    virtual void        setValue( const String & aStrg );

    // Methods of the object
    virtual void        focus();
    virtual void        blur();
    virtual void        select();

    // Events (C++ --> Java)
    //----------------------
    // none!

    // construktor
    ImpSjJScriptPasswordObject( VCControl* _pControl, SjJSbxObject * p );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class ImpSjJScriptCheckboxObject : public SjJScriptCheckboxObject, ImpSjJScriptControlBase
{
  public:
    // Methods from Java --> C++ (for properties)
    //--------------------------------------------
    // Base class properties, are passed through to ImpSjJScriptControlBase
    virtual String      getName() const;
    virtual void        setName( const String & aStrg );
    virtual String      getValue() const;
    virtual String      getType() const;

    virtual sal_Bool        getChecked() const;
    virtual void        setChecked( sal_Bool bCheck );
    virtual sal_Bool        getDefaultChecked() const;
    virtual void        setDefaultChecked( sal_Bool bCheck );
    virtual void        setValue( const String & aStrg );

    // Methods of the object
    virtual void        click();

    // Events (C++ --> Java)
    //----------------------
    // New event interface with function objects
    virtual SjJScriptFunctionObject* getOnClick_Fct() const;
    virtual void setOnClick_Fct( SjJScriptFunctionObject* pFunctionObject );
    virtual String      getOnClick() const;
    virtual void        setOnClick( const String & aSourceStrg );
            void        onClick();

    // construktor
    ImpSjJScriptCheckboxObject( VCControl* _pControl, SjJSbxObject * p );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Wrapper class for RadioArray
class ImpSjJScriptRadioArray: public SjJSbxArrayObject
{
    VCRadioManager* pMgr;
public:
    sal_Int32 ImpSjJScriptRadioArray::getLength() const;
    SjJSbxObject* ImpSjJScriptRadioArray::getElement( sal_Int32 nIndex ) const;
    SjJSbxObject* ImpSjJScriptRadioArray::getElement_String( const String & aName ) const;

    // construktor
    ImpSjJScriptRadioArray::ImpSjJScriptRadioArray( VCRadioManager* _pMgr, SjJSbxObject * p );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class ImpSjJScriptRadioObject : public SjJScriptRadioObject, ImpSjJScriptControlBase
{
  public:
    // Methods from Java --> C++ (for properties)
    //--------------------------------------------
    // Base class properties, are passed through to ImpSjJScriptControlBase
    virtual String      getName() const;
    virtual void        setName( const String & aStrg );
    virtual String      getValue() const;
    virtual String      getType() const;

    // Properties of SjJScriptCheckboxObject
    virtual sal_Bool        getChecked() const;
    virtual void        setChecked( sal_Bool bCheck );
    virtual sal_Bool        getDefaultChecked() const;
    virtual void        setDefaultChecked( sal_Bool bCheck );
    virtual void        setValue( const String & aStrg );

    // Methods of the object
    virtual void        click();

    // Events (C++ --> Java)
    // New event interface with function objects
    virtual SjJScriptFunctionObject* getOnClick_Fct() const;
    virtual void setOnClick_Fct( SjJScriptFunctionObject* pFunctionObject );
    virtual String      getOnClick() const;
    virtual void        setOnClick( const String & aSourceStrg );
            void        onClick();

    // Constructor
    ImpSjJScriptRadioObject( VCControl* _pControl, SjJSbxObject * p );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class ImpSjJScriptSelectObject : public SjJScriptSelectObject, ImpSjJScriptControlBase
{
  public:
    // Methods from Java --> C++ (for properties)
    //--------------------------------------------
    // Base class properties, are passed through to ImpSjJScriptControlBase
    virtual String      getName() const;
    virtual void        setName( const String & aStrg );
    virtual String      getValue() const;
    virtual String      getType() const;

    virtual sal_Int32       getLength();
    virtual sal_Int32       getSelectedIndex() const;
    virtual void        setSelectedIndex( sal_Int32 nNo );

    // Methods of the object
    virtual void        focus();
    virtual void        blur();

    // Events (C++ --> Java)
    //----------------------
    // New event interface with function objects
    virtual SjJScriptFunctionObject* getOnBlur_Fct() const;
    virtual void setOnBlur_Fct( SjJScriptFunctionObject* pFunctionObject );
    virtual SjJScriptFunctionObject* getOnChange_Fct() const;
    virtual void setOnChange_Fct( SjJScriptFunctionObject* pFunctionObject );
    virtual SjJScriptFunctionObject* getOnFocus_Fct() const;
    virtual void setOnFocus_Fct( SjJScriptFunctionObject* pFunctionObject );
    virtual String      getOnBlur() const;
    virtual void        setOnBlur( const String & aSourceStrg );
            void        onBlur();
    virtual String      getOnChange() const;
    virtual void        setOnChange( const String & aSourceStrg );
            void        onChange();
    virtual String      getOnFocus() const;
    virtual void        setOnFocus( const String & aSourceStrg );
            void        onFocus();

    // SjJSbxArrayObject methods
    virtual sal_Int32           getLength() const;
    // Access through the index
    virtual SjJSbxObject *  getElement( sal_Int32 nIndex ) const;

    // Constructor
    ImpSjJScriptSelectObject( VCControl* _pControl, SjJSbxObject * p );

    // for setting entries of the object (to be called from Java)
    virtual void        setOption( sal_Int32 nIndex, const String & aOptionText,
                                   const String & aOptionValue,
                                   sal_Bool bDefaultSelected, sal_Bool bSelected );

    // interface for the properties of the option objects
    virtual sal_Bool        getOptionDefaultSelected( sal_Int32 nIndex ) const;
    virtual void        setOptionDefaultSelected( sal_Bool bSelected, sal_Int32 nIndex );
    virtual sal_Bool        getOptionSelected( sal_Int32 nIndex ) const;
    virtual void        setOptionSelected( sal_Bool bSelected, sal_Int32 nIndex );
    virtual String      getOptionText( sal_Int32 nIndex ) const;
    virtual void        setOptionText( const String & sText, sal_Int32 nIndex );
    virtual void        setOptionValue( const String & sText, sal_Int32 nIndex );
    virtual String      getOptionValue( sal_Int32 nIndex ) const;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class ImpSjJScriptOptionObject : public SjJScriptOptionObject, ImpSjJScriptControlBase
{
    sal_uInt16              nListBoxIndex;
  public:
    // Methods from Java --> C++ (for properties)
    //--------------------------------------------
    virtual sal_Bool        getdefaultSelected() const;
    virtual void        setdefaultSelected( sal_Bool bSelected );
    virtual sal_Int32       getIndex() const;
    virtual sal_Bool        getSelected() const;
    virtual void        setSelected( sal_Bool bSelected );

    virtual String      getText() const;
    virtual void        setText( const String & sText );
    virtual String      getValue() const;
    virtual void        setValue( const String & aStrg );


    // DUMMY methods
    virtual String getName() const { return "";}
    virtual void setName( const String &s) { }
    virtual String getType() const { return "";}
    // Methods of the object
    // none !

    // Events (C++ --> Java)
    //----------------------
    // none !

    // Constructor
    ImpSjJScriptOptionObject( VCControl* _pControl, SjJSbxObject * p, sal_uInt16 _nListBoxIndex );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Separate class for forms array
class SjJSbxFormsArray: public SfxJSArray
{
    VCManager* pManager;

  public:
    // SjJSbxArrayObject methods
    virtual sal_Int32           getLength() const;
    // access through the index
    virtual SjJSbxObject *  getElement( sal_Int32 nIndex ) const;
    // access through the name
    // cannot overload native java methods (yet)
    virtual SjJSbxObject *  getElement_String( const String & aName ) const;

    // Constructor
    SjJSbxFormsArray( VCManager * _pManager, SjJSbxObject * p );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class ImpSjJScriptTextObject : public SjJScriptTextObject, ImpSjJScriptControlBase
{
  public:
    // Methods from Java --> C++ (for properties)
    //--------------------------------------------
    // Base class properties, are passed through to ImpSjJScriptControlBase
    virtual String      getName() const;
    virtual void        setName( const String & aStrg );
    virtual String      getValue() const;
    virtual String      getType() const;

    virtual String      getDefaultValue() const;
    virtual void        setDefaultValue( const String & aStrg );
    virtual void        setValue( const String & aStrg );

    // Methods of the object
    virtual void        focus();
    virtual void        blur();
    virtual void        select();

    // Events (C++ --> Java)
    //----------------------
    // New event interface with function objects
    virtual SjJScriptFunctionObject* getOnBlur_Fct() const;
    virtual void setOnBlur_Fct( SjJScriptFunctionObject* pFunctionObject );
    virtual SjJScriptFunctionObject* getOnChange_Fct() const;
    virtual void setOnChange_Fct( SjJScriptFunctionObject* pFunctionObject );
    virtual SjJScriptFunctionObject* getOnFocus_Fct() const;
    virtual void setOnFocus_Fct( SjJScriptFunctionObject* pFunctionObject );
    virtual SjJScriptFunctionObject* getOnSelect_Fct() const;
    virtual void setOnSelect_Fct( SjJScriptFunctionObject* pFunctionObject );
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

    // Constructor
    ImpSjJScriptTextObject( VCControl* _pControl, SjJSbxObject * p );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class ImpSjJScriptTextareaObject : public SjJScriptTextareaObject, ImpSjJScriptControlBase
{
  public:
    // Methods from Java --> C++ (for properties)
    //--------------------------------------------
    // Base class properties, are passed through to ImpSjJScriptControlBase
    virtual String      getName() const;
    virtual void        setName( const String & aStrg );
    virtual String      getValue() const;
    virtual String      getType() const;

    virtual String      getDefaultValue() const;
    virtual void        setDefaultValue( const String & aStrg );
    virtual void        setValue( const String & aStrg );

    // Methods of the object
    virtual void        focus();
    virtual void        blur();
    virtual void        select();

    // Events (C++ --> Java)
    //----------------------
    // New event interface with function objects
    virtual SjJScriptFunctionObject* getOnBlur_Fct() const;
    virtual void setOnBlur_Fct( SjJScriptFunctionObject* pFunctionObject );
    virtual SjJScriptFunctionObject* getOnChange_Fct() const;
    virtual void setOnChange_Fct( SjJScriptFunctionObject* pFunctionObject );
    virtual SjJScriptFunctionObject* getOnFocus_Fct() const;
    virtual void setOnFocus_Fct( SjJScriptFunctionObject* pFunctionObject );
    virtual SjJScriptFunctionObject* getOnSelect_Fct() const;
    virtual void setOnSelect_Fct( SjJScriptFunctionObject* pFunctionObject );
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

    // Constructor
    ImpSjJScriptTextareaObject( VCControl* _pControl, SjJSbxObject * p );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class ImpSjJScriptSubmitObject : public SjJScriptSubmitObject, ImpSjJScriptControlBase
{
    // identically equal to ButtonObject
  public:
    // Methods from Java --> C++ (for properties)
    //--------------------------------------------
    // Base class properties, are passed through to ImpSjJScriptControlBase
    virtual String      getName() const;
    virtual void        setName( const String & aStrg );
    virtual String      getValue() const;
    virtual void        setValue( const String & aStrg );
    virtual String      getType() const;

    // Methods of the object
    virtual void        click();

    // Events (C++ --> Java)
    //----------------------
    // New event interface with function objects
    virtual SjJScriptFunctionObject* getOnClick_Fct() const;
    virtual void setOnClick_Fct( SjJScriptFunctionObject* pFunctionObject );
    virtual String      getOnClick() const;
    virtual void        setOnClick( const String & aSourceStrg );
            void        onClick();

    // Constructor
    ImpSjJScriptSubmitObject( VCControl* _pControl, SjJSbxObject * p );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class ImpSjJScriptResetObject : public SjJScriptResetObject, ImpSjJScriptControlBase
{
    // identically equal to ButtonObject
  public:
    // Methods from Java --> C++ (for properties)
    //--------------------------------------------
    // Base class properties, are passed through to ImpSjJScriptControlBase
    virtual String      getName() const;
    virtual void        setName( const String & aStrg );
    virtual String      getValue() const;
    virtual void        setValue( const String & aStrg );
    virtual String      getType() const;

    // Methods of the object
    virtual void        click();

    // Events (C++ --> Java)
    //----------------------
    // New event interface with function objects
    virtual SjJScriptFunctionObject* getOnClick_Fct() const;
    virtual void setOnClick_Fct( SjJScriptFunctionObject* pFunctionObject );
    virtual String      getOnClick() const;
    virtual void        setOnClick( const String & aSourceStrg );
            void        onClick();

    // Constructor
    ImpSjJScriptResetObject( VCControl* _pControl, SjJSbxObject * p );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class ImpSjJScriptHiddenObject : public SjJScriptHiddenObject, ImpSjJScriptControlBase
{
  public:
    // Methods from Java --> C++ (for properties)
    //--------------------------------------------
    // Base class properties, are passed through to ImpSjJScriptControlBase
    virtual String      getName() const;
    virtual void        setName( const String & aStrg );
    virtual String      getValue() const;
    virtual String      getType() const;

    virtual void        setValue( const String & aStrg );

    // Methods of the object
    // none !

    // Events (C++ --> Java)
    //----------------------
    // none !

    // Constructor
    ImpSjJScriptHiddenObject( VCControl* _pControl, SjJSbxObject * p );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class ImpSjJScriptFileUploadObject : public SjJScriptFileUploadObject, ImpSjJScriptControlBase
{
  public:
    // Methods from Java --> C++ (for properties)
    //--------------------------------------------
    // Base class properties, are passed through to ImpSjJScriptControlBase
    virtual String      getName() const;
    virtual void        setName( const String & aStrg );
    virtual String      getValue() const;
    virtual String      getType() const;

    // Methods of the object
    // none !

    // Events (C++ --> Java)
    //----------------------
    // none !

    // Constructor
    ImpSjJScriptFileUploadObject( VCControl* _pControl, SjJSbxObject * p );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class ImpSjJScriptFormObject : public SjJScriptFormObject
{
    VCForm* pVCForm;

  public:
    // Methods from Java --> C++ (for properties)
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

    // SjJSbxArrayObject methods
    sal_Int32               getLength() const;
    SjJSbxObject*       getElement( sal_Int32 nIndex ) const;
    SjJSbxObject*       getElement_String( const String & aName ) const;

    // Methods of the object
    virtual void        submit();
    virtual void        reset();

    // Events (C++ --> Java)
    //----------------------
    // New event interface with function objects
    virtual SjJScriptFunctionObject* getOnSubmit_Fct() const;
    virtual void setOnSubmit_Fct( SjJScriptFunctionObject* pFunctionObject );
    virtual SjJScriptFunctionObject* getOnReset_Fct() const;
    virtual void setOnReset_Fct( SjJScriptFunctionObject* pFunctionObject );
    virtual String      getOnSubmit() const;
    virtual void        setOnSubmit( const String & aSourceStrg );
            void        onSubmit();
    virtual String      getOnReset() const;
    virtual void        setOnReset( const String & aSourceStrg );
            void        onReset();

    // Constructor
    ImpSjJScriptFormObject( VCForm* _pVCForm, SjJSbxObject * p );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
