/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVX_SOURCE_INC_GRIDCELL_HXX
#define INCLUDED_SVX_SOURCE_INC_GRIDCELL_HXX

#include <svx/gridctrl.hxx>

#include "sqlparserclient.hxx"

#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/form/XBoundControl.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/awt/XListBox.hpp>
#include <com/sun/star/awt/XComboBox.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XCheckBox.hpp>
#include <com/sun/star/awt/XButton.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/form/XChangeBroadcaster.hpp>
#include <com/sun/star/awt/XWindow.hpp>

#include <comphelper/propmultiplex.hxx>
#include <cppuhelper/component.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <tools/diagnose_ex.h>
#include <tools/rtti.hxx>

class DbCellControl;
class Edit;
class FmXGridCell;

class FmMutexHelper
{
protected:
    ::osl::Mutex    m_aMutex;
};


// DbGridColumn, Spaltenbeschreibung

class DbGridColumn
{
    friend class DbGridControl;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xField;       // Verbindung zum Datenbankfeld
    ::svt::CellControllerRef m_xController; // Struktur zum Verwalten der Controls fuer eine Spalte
                                        // diese wird von der DbBrowseBox auf die jeweiligen Zellen
                                        // einer Spalte positioniert
    FmXGridCell*                m_pCell;

protected:
    DbGridControl&      m_rParent;

private:
    sal_Int32               m_nLastVisibleWidth;    // nur gueltig, wenn m_bHidden == sal_True
    sal_Int32               m_nFormatKey;
    sal_Int16               m_nFieldType;
    sal_Int16               m_nTypeId;
    sal_uInt16              m_nId;
    sal_Int16               m_nFieldPos;
    sal_Int16               m_nAlign;                       // wird mit TXT_ALIGN_LEFT .... angegeben
    bool                m_bReadOnly : 1;
    bool                m_bAutoValue : 1;
    bool                m_bInSave : 1;
    bool                m_bNumeric : 1;
    bool                m_bObject : 1;  // Verweist die Column auf ein Object Datentyp?
    bool                m_bHidden : 1;
    bool                m_bLocked : 1;
    bool                m_bDateTime : 1;

    static ::svt::CellControllerRef s_xEmptyController;
        // used by locked columns
public:
    DbGridColumn(sal_uInt16 _nId, DbGridControl& rParent)
        :m_pCell(NULL)
        ,m_rParent(rParent)
        ,m_nLastVisibleWidth(-1)
        ,m_nFormatKey(0)
        ,m_nFieldType(0)
        ,m_nTypeId(0)
        ,m_nId(_nId)
        ,m_nFieldPos(-1)
        ,m_nAlign(::com::sun::star::awt::TextAlign::LEFT)
        ,m_bReadOnly(false)
        ,m_bAutoValue(false)
        ,m_bInSave(false)
        ,m_bNumeric(false)
        ,m_bObject(false)
        ,m_bHidden(false)
        ,m_bLocked(false)
        ,m_bDateTime(false)
    {
    }

    ~DbGridColumn();

    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& getModel() const { return m_xModel; }
    void  setModel(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  _xModel);


    sal_uInt16  GetId() const {return m_nId;}
    bool    IsReadOnly() const {return m_bReadOnly;}
    bool    IsAutoValue() const {return m_bAutoValue;}
    bool    IsUpdating() const {return m_bInSave;}
    sal_Int16   GetAlignment() const {return m_nAlign;}
    sal_Int16   GetType() const {return m_nFieldType;}
    sal_Int16   GetFieldPos() const {return m_nFieldPos; }
    bool    IsNumeric() const {return m_bNumeric;}
    bool    IsDateTime() const {return m_bDateTime;}
    bool    IsObject() const {return m_bObject;}
    bool    IsHidden() const {return m_bHidden;}
    sal_Int32   GetKey() const {return m_nFormatKey;}
    const   ::svt::CellControllerRef& GetController() const {return m_bLocked ? s_xEmptyController : m_xController;}
    const   ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& GetField() const {return m_xField;}
    DbGridControl& GetParent() const {return m_rParent;}
    FmXGridCell* GetCell() const {return m_pCell;}

    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >  GetCurrentFieldValue() const;

    //      Zeichnen eines Feldes an einer Position, ist ein View gesetzt
    //      uebernimmt dieser das Zeichnen, z.B. fuer CheckBoxen
    void    Paint(OutputDevice& rDev,
                  const Rectangle& rRect,
                  const DbGridRow* pRow,
                  const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);


    //      Inititialierung im alive mode
    //      Ist kein ColumnController gesetzt, wird eine DefaultInitialisierung
    //      vorgenommen
    void    CreateControl(sal_Int32 _nFieldPos, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xField, sal_Int32 nTypeId);
    void    UpdateControl()
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xField(m_xField);
                CreateControl(m_nFieldPos, xField, m_nTypeId);
            }

    //      Editieren einer Zelle
    void    UpdateFromField(const DbGridRow* pRow, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);
    bool    Commit();

    //      freigeben aller Daten, die fuer den AliveMode noetig sind
    void    Clear();

    OUString  GetCellText(const DbGridRow* pRow, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) const;
    OUString  GetCellText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& xField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) const;

    void    SetReadOnly(bool bRead){m_bReadOnly = bRead;}
    void    SetObject(sal_Int16 nPos) {m_bObject = m_bReadOnly = true; m_nFieldPos = nPos;}

    void    ImplInitWindow( vcl::Window& rParent, const InitWindowFacet _eInitWhat );

    // Properties, die auf den ::com::sun::star::frame::Controller durchschlagen koennen
    sal_Int16   SetAlignment(sal_Int16 _nAlign);
        // if _nAlign is -1, the alignment is calculated from the type of the field we are bound to
        // the value really set is returned
    sal_Int16   SetAlignmentFromModel(sal_Int16 nStandardAlign);
        // set the alignment according to the "Align"-property of m_xModel, use the given standard
        // alignment if the property if void, return the really set alignment

    // column locking
    bool    isLocked() const { return m_bLocked; }
    void    setLock(bool _bLock);

private:
    /** attaches or detaches our cell object to the SctriptEventAttacherManager implemented
        by our model's parent
    */
    void    impl_toggleScriptManager_nothrow( bool _bAttach );
};


// DbCellControl, liefert die Daten fuer einen CellController
// wird in der Regel nur f\FCr komplexe Controls wie z.B ComboBoxen
// benoetigt

class DbCellControl
        :public FmMutexHelper           // _before_ the listener, so the listener is to be destroyed first!
        ,public ::comphelper::OPropertyChangeListener
{
private:
    ::comphelper::OPropertyChangeMultiplexer*   m_pModelChangeBroadcaster;
    ::comphelper::OPropertyChangeMultiplexer*   m_pFieldChangeBroadcaster;

private:
    bool                    m_bTransparent : 1;
    bool                    m_bAlignedController : 1;
    bool                    m_bAccessingValueProperty : 1;

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >
                                m_xCursor;

protected:
    DbGridColumn&               m_rColumn;
    vcl::Window*                     m_pPainter;
    vcl::Window*                     m_pWindow;

protected:
    // attribute access
    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& getCursor() const { return m_xCursor; }

    // control transparency
    inline  bool    isTransparent( ) const { return m_bTransparent; }
    inline  void        setTransparent( bool _bSet ) { m_bTransparent = _bSet; }

    // control alignment
    inline  void        setAlignedController( bool _bAlign = true ) { m_bAlignedController = _bAlign; }


    /** determined whether or not the value property is locked
    @see lockValueProperty
    */
    inline  bool    isValuePropertyLocked() const;

    /** locks the listening at the value property.
        <p>This means that every subsequent change now done on the value property of the model ("Text", or "Value",
        or whatever) is then ignored.<br/>
        This base class uses this setting in <method>Commit</method>.</p>
    @precond
        Value locking can't be nested
    @see unlockValueProperty
    */
    inline  void        lockValueProperty();
    /** unlocks the listening at the value property
    @see lockValueProperty
    */
    inline  void        unlockValueProperty();

protected:
    // adds the given property to the list of properties which we listen for
    void    doPropertyListening( const OUString& _rPropertyName );

    // called whenever a property which affects field settings in general is called
    // you should overwrite this method for every property you add yourself as listener to
    // with doPropertyListening
    virtual void    implAdjustGenericFieldSetting( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel );

    // called by _propertyChanged if a property which denotes the column value has changed
    void    implValuePropertyChanged( );


public:
    TYPEINFO();
    DbCellControl(DbGridColumn& _rColumn, bool _bText = true);
    virtual ~DbCellControl();


    vcl::Window& GetWindow() const
    {
        ENSURE_OR_THROW( m_pWindow, "no window" );
        return *m_pWindow;
    }

    // control alignment
    inline  bool    isAlignedController() const { return m_bAlignedController; }
            void        AlignControl(sal_Int16 nAlignment);

    void SetTextLineColor();
    void SetTextLineColor(const Color& _rColor);

    // Initialisieren bevor ein Control angezeigt wird
    virtual void Init( vcl::Window& rParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor );
    virtual ::svt::CellControllerRef CreateController() const = 0;

    // Schreiben des Wertes in das Model
    bool Commit();

    // Formatting the field data to output text
    virtual OUString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL) = 0;

    virtual void Update(){}
    // Refresh the control by the field data
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) = 0;

    // Painten eines Zellinhalts im vorgegeben Rechteck
    virtual void PaintFieldToCell( OutputDevice& rDev, const Rectangle& rRect, const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);
    virtual void PaintCell( OutputDevice& _rDev, const Rectangle& _rRect );

    void  ImplInitWindow( vcl::Window& rParent, const InitWindowFacet _eInitWhat );

    double GetValue(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) const;

protected:
    void    invalidatedController();

    /** commits the content of the control (e.g. the text of an edit field) into the column model
        (e.g. the "Text" property of the model).
        <p>To be overwritten in derived classes.</p>
    @see updateFromModel
    */
    virtual bool commitControl( ) = 0;

    /** updates the current content of the control (e.g. the text of an edit field) from the column model
        (e.g. the "Text" property of the model).
        <p>To be overwritten in derived classes.</p>
    @precond
        NULL != _rxModel
    @precond
        NULL != m_pWindow

    @see commitControl
    */
    virtual void    updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel ) = 0;

protected:
// OPropertyChangeListener
    virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

private:
    void implDoPropertyListening( const OUString& _rPropertyName, bool _bWarnIfNotExistent = true );

    /// updates the "readonly" setting on m_pWindow, according to the respective property value in the given model
    void implAdjustReadOnly( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel,bool i_bReadOnly );

    /// updates the "enabled" setting on m_pWindow, according to the respective property value in the given model
    void implAdjustEnabled( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel );
};



inline  bool    DbCellControl::isValuePropertyLocked() const
{
    return m_bAccessingValueProperty;
}


inline  void        DbCellControl::lockValueProperty()
{
    OSL_ENSURE( !isValuePropertyLocked(), "DbCellControl::lockValueProperty: not to be nested!" );
    m_bAccessingValueProperty = true;
}


inline  void        DbCellControl::unlockValueProperty()
{
    OSL_ENSURE( isValuePropertyLocked(), "DbCellControl::lockValueProperty: not locked so far!" );
    m_bAccessingValueProperty = false;
}


/** a field which is bound to a column which supports the MaxTextLen property
*/
class DbLimitedLengthField : public DbCellControl
{
public:
    TYPEINFO_OVERRIDE();

protected:
    DbLimitedLengthField( DbGridColumn& _rColumn );

protected:
    // DbCellControl
    virtual void implAdjustGenericFieldSetting( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel ) SAL_OVERRIDE;

protected:
    void implSetMaxTextLen( sal_Int16 _nMaxLen )
    {
        implSetEffectiveMaxTextLen( _nMaxLen ? _nMaxLen : EDIT_NOLIMIT );
    }
    virtual void implSetEffectiveMaxTextLen( sal_Int32 _nMaxLen );
};


class DbTextField : public DbLimitedLengthField
{
    ::svt::IEditImplementation* m_pEdit;
    ::svt::IEditImplementation* m_pPainterImplementation;
    bool                    m_bIsSimpleEdit;

protected:
    virtual ~DbTextField( );

public:
    TYPEINFO_OVERRIDE();
    DbTextField(DbGridColumn& _rColumn);

    ::svt::IEditImplementation* GetEditImplementation() { return m_pEdit; }
    bool                    IsSimpleEdit() const { return m_bIsSimpleEdit; }

    virtual void Init( vcl::Window& rParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor ) SAL_OVERRIDE;
    virtual OUString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL) SAL_OVERRIDE;
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) SAL_OVERRIDE;
    virtual ::svt::CellControllerRef CreateController() const SAL_OVERRIDE;
    virtual void PaintFieldToCell( OutputDevice& _rDev, const Rectangle& _rRect,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter ) SAL_OVERRIDE;

protected:
    // DbCellControl
    virtual bool        commitControl( ) SAL_OVERRIDE;
    virtual void        updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel ) SAL_OVERRIDE;
    // DbLimitedLengthField
    virtual void        implSetEffectiveMaxTextLen( sal_Int32 _nMaxLen ) SAL_OVERRIDE;
};


class DbFormattedField : public DbLimitedLengthField
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >  m_xSupplier;
    sal_Int16                       m_nKeyType;


public:
    TYPEINFO_OVERRIDE();
    DbFormattedField(DbGridColumn& _rColumn);
    virtual ~DbFormattedField();


    virtual void Init( vcl::Window& rParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor ) SAL_OVERRIDE;
    virtual OUString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL) SAL_OVERRIDE;
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) SAL_OVERRIDE;
    virtual ::svt::CellControllerRef CreateController() const SAL_OVERRIDE;

protected:
    // DbCellControl
    virtual bool        commitControl( ) SAL_OVERRIDE;
    virtual void        updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel ) SAL_OVERRIDE;

    // OPropertyChangeListener
    virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
};


class DbCheckBox : public DbCellControl
{
public:
    TYPEINFO_OVERRIDE();
    DbCheckBox(DbGridColumn& _rColumn);

    virtual void Init( vcl::Window& rParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor ) SAL_OVERRIDE;
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) SAL_OVERRIDE;
    virtual ::svt::CellControllerRef CreateController() const SAL_OVERRIDE;
    virtual void PaintFieldToCell(OutputDevice& rDev, const Rectangle& rRect,
                          const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField,
                          const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) SAL_OVERRIDE;
    virtual OUString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL) SAL_OVERRIDE;

protected:
    // DbCellControl
    virtual bool        commitControl( ) SAL_OVERRIDE;
    virtual void        updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel ) SAL_OVERRIDE;
};


class DbComboBox : public DbCellControl
{

public:
    TYPEINFO_OVERRIDE();
    DbComboBox(DbGridColumn& _rColumn);

    virtual void Init( vcl::Window& rParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor ) SAL_OVERRIDE;
    virtual OUString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL) SAL_OVERRIDE;
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) SAL_OVERRIDE;
    virtual ::svt::CellControllerRef CreateController() const SAL_OVERRIDE;

    void SetList(const ::com::sun::star::uno::Any& rItems);

protected:
    // DbCellControl
    virtual bool        commitControl( ) SAL_OVERRIDE;
    virtual void        updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel ) SAL_OVERRIDE;

    virtual void        implAdjustGenericFieldSetting( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel ) SAL_OVERRIDE;

    // OPropertyChangeListener
    virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
};


class DbListBox     :public DbCellControl
{
    bool              m_bBound  : 1;
    ::com::sun::star::uno::Sequence< OUString > m_aValueList;

public:
    TYPEINFO_OVERRIDE();
    DbListBox(DbGridColumn& _rColumn);

    virtual void Init( vcl::Window& rParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor ) SAL_OVERRIDE;
    virtual OUString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL) SAL_OVERRIDE;
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) SAL_OVERRIDE;
    virtual ::svt::CellControllerRef CreateController() const SAL_OVERRIDE;

    void SetList(const ::com::sun::star::uno::Any& rItems);

protected:
    // DbCellControl
    virtual bool        commitControl( ) SAL_OVERRIDE;
    virtual void        updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel ) SAL_OVERRIDE;

    virtual void        implAdjustGenericFieldSetting( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel ) SAL_OVERRIDE;

    // OPropertyChangeListener
    virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
};


class DbPatternField : public DbCellControl
{
public:
    TYPEINFO_OVERRIDE();
    DbPatternField( DbGridColumn& _rColumn, const css::uno::Reference<css::uno::XComponentContext>& _rContext );
    virtual void Init( vcl::Window& rParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor ) SAL_OVERRIDE;
    virtual OUString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL) SAL_OVERRIDE;
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) SAL_OVERRIDE;
    virtual ::svt::CellControllerRef CreateController() const SAL_OVERRIDE;

protected:
    /// DbCellControl
    virtual bool        commitControl( ) SAL_OVERRIDE;
    virtual void        updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel ) SAL_OVERRIDE;

    virtual void        implAdjustGenericFieldSetting( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel ) SAL_OVERRIDE;

private:
    OUString  impl_formatText(const OUString& _rText);

private:
    ::std::unique_ptr< ::dbtools::FormattedColumnValue >  m_pValueFormatter;
    ::std::unique_ptr< ::dbtools::FormattedColumnValue >  m_pPaintFormatter;
    css::uno::Reference<css::uno::XComponentContext>    m_xContext;
};


class DbSpinField : public DbCellControl
{
private:
    sal_Int16   m_nStandardAlign;

public:
    TYPEINFO_OVERRIDE();

protected:
    DbSpinField( DbGridColumn& _rColumn, sal_Int16 _nStandardAlign = com::sun::star::awt::TextAlign::RIGHT );

public:
    virtual void                        Init( vcl::Window& rParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& _rxCursor ) SAL_OVERRIDE;
    virtual ::svt::CellControllerRef    CreateController() const SAL_OVERRIDE;

protected:
    virtual SpinField*  createField(
                            vcl::Window* _pParent,
                            WinBits _nFieldStyle,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel
                        ) = 0;
};


class DbDateField : public DbSpinField
{
public:
    TYPEINFO_OVERRIDE();
    DbDateField(DbGridColumn& _rColumn);
    virtual OUString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL) SAL_OVERRIDE;
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) SAL_OVERRIDE;

protected:
    // DbCellControl
    virtual bool        commitControl( ) SAL_OVERRIDE;
    virtual void        updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel ) SAL_OVERRIDE;

    // DbSpinField
    virtual SpinField*  createField(
                            vcl::Window* _pParent,
                            WinBits _nFieldStyle,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel
                        ) SAL_OVERRIDE;

    /// initializes everything which relates to the properties describing the numeric behaviour
    virtual void    implAdjustGenericFieldSetting( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel ) SAL_OVERRIDE;
};


class DbTimeField : public DbSpinField
{
public:
    TYPEINFO_OVERRIDE();
    DbTimeField(DbGridColumn& _rColumn);
    virtual OUString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL) SAL_OVERRIDE;
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) SAL_OVERRIDE;

protected:
    // DbCellControl
    virtual bool        commitControl( ) SAL_OVERRIDE;
    virtual void        updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel ) SAL_OVERRIDE;

    // DbSpinField
    virtual SpinField*  createField(
                            vcl::Window* _pParent,
                            WinBits _nFieldStyle,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel
                        ) SAL_OVERRIDE;

    /// initializes everything which relates to the properties describing the numeric behaviour
    virtual void    implAdjustGenericFieldSetting( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel ) SAL_OVERRIDE;
};


class DbCurrencyField : public DbSpinField
{
    sal_Int16  m_nScale;

public:
    TYPEINFO_OVERRIDE();
    DbCurrencyField(DbGridColumn& _rColumn);
    virtual OUString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL) SAL_OVERRIDE;
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) SAL_OVERRIDE;

    double GetCurrency(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) const;

protected:
    // DbCellControl
    virtual bool        commitControl( ) SAL_OVERRIDE;
    virtual void        updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel ) SAL_OVERRIDE;

    // DbSpinField
    virtual SpinField*  createField(
                            vcl::Window* _pParent,
                            WinBits _nFieldStyle,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel
                        ) SAL_OVERRIDE;

    /// initializes everything which relates to the properties describing the numeric behaviour
    virtual void    implAdjustGenericFieldSetting( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel ) SAL_OVERRIDE;
};


class DbNumericField : public DbSpinField
{
public:
    TYPEINFO_OVERRIDE();
    DbNumericField(DbGridColumn& _rColumn);
    virtual OUString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL) SAL_OVERRIDE;
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) SAL_OVERRIDE;

protected:
    // DbCellControl
    virtual bool        commitControl( ) SAL_OVERRIDE;
    virtual void        updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel ) SAL_OVERRIDE;

    // DbSpinField
    virtual SpinField*  createField(
                            vcl::Window* _pParent,
                            WinBits _nFieldStyle,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel
                        ) SAL_OVERRIDE;

    /// initializes everything which relates to the properties describing the numeric behaviour
    void    implAdjustGenericFieldSetting( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel ) SAL_OVERRIDE;
};


class DbFilterField
        :public DbCellControl
        ,public ::svxform::OSQLParserClient
{
    ::com::sun::star::uno::Sequence< OUString >  m_aValueList;
    OUString   m_aText;
    Link    m_aCommitLink;
    sal_Int16   m_nControlClass;
    bool    m_bFilterList : 1;
    bool    m_bFilterListFilled : 1;
    bool    m_bBound : 1;

public:
    TYPEINFO_OVERRIDE();
    DbFilterField(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext, DbGridColumn& _rColumn);
    virtual ~DbFilterField();

    virtual void Init( vcl::Window& rParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor ) SAL_OVERRIDE;
    virtual ::svt::CellControllerRef CreateController() const SAL_OVERRIDE;
    virtual void PaintCell(OutputDevice& rDev, const Rectangle& rRect) SAL_OVERRIDE;
    virtual void Update() SAL_OVERRIDE;
    virtual OUString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL) SAL_OVERRIDE;
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) SAL_OVERRIDE;

    const OUString& GetText() const {return m_aText;}
    void SetText(const OUString& rText);

    void SetCommitHdl( const Link& rLink ) { m_aCommitLink = rLink; }
    const Link& GetCommitHdl() const { return m_aCommitLink; }

protected:

    // DbCellControl
    virtual bool        commitControl( ) SAL_OVERRIDE;
    virtual void        updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel ) SAL_OVERRIDE;

protected:
    void SetList(const ::com::sun::star::uno::Any& rItems, bool bComboBox);
    void CreateControl(vcl::Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xModel);
    DECL_LINK( OnClick, void* );
};


// Base class providing the access to a grid cell

typedef ::cppu::ImplHelper2 <   ::com::sun::star::awt::XControl
                            ,   ::com::sun::star::form::XBoundControl
                            >   FmXGridCell_Base;
typedef ::cppu::ImplHelper1 <   ::com::sun::star::awt::XWindow
                            >   FmXGridCell_WindowBase;
class FmXGridCell   :public ::cppu::OComponentHelper
                    ,public FmXGridCell_Base
                    ,public FmXGridCell_WindowBase
{
protected:
    ::osl::Mutex        m_aMutex;
    DbGridColumn*       m_pColumn;
    DbCellControl*      m_pCellControl;

private:
    ::cppu::OInterfaceContainerHelper   m_aWindowListeners;
    ::cppu::OInterfaceContainerHelper   m_aFocusListeners;
    ::cppu::OInterfaceContainerHelper   m_aKeyListeners;
    ::cppu::OInterfaceContainerHelper   m_aMouseListeners;
    ::cppu::OInterfaceContainerHelper   m_aMouseMotionListeners;

protected:
    virtual ~FmXGridCell();

public:
    TYPEINFO();
    FmXGridCell( DbGridColumn* pColumn, DbCellControl* pControl );
    void init();

    DECLARE_UNO3_AGG_DEFAULTS(FmXGridCell, OComponentHelper)
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    void SetTextLineColor();
    void SetTextLineColor(const Color& _rColor);

// XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// OComponentHelper
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

// ::com::sun::star::lang::XComponent
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE {OComponentHelper::dispose();}
    virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener)throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE       { OComponentHelper::addEventListener(aListener);}
    virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener)throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE        { OComponentHelper::removeEventListener(aListener);}

// ::com::sun::star::awt::XControl
    virtual void SAL_CALL setContext(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& /*Context*/) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE {}
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL getContext() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL createPeer(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& /*Toolkit*/, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& /*Parent*/) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE {}

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > SAL_CALL getPeer() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE {return ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > ();}
    virtual sal_Bool SAL_CALL setModel(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& /*Model*/) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE {return sal_False;}
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > SAL_CALL getModel() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XView > SAL_CALL getView() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE {return ::com::sun::star::uno::Reference< ::com::sun::star::awt::XView > ();}
    virtual void SAL_CALL setDesignMode(sal_Bool /*bOn*/) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE {}
    virtual sal_Bool SAL_CALL isDesignMode() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE {return sal_False;}
    virtual sal_Bool SAL_CALL isTransparent() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE {return sal_False;}

// ::com::sun::star::form::XBoundControl
    virtual sal_Bool SAL_CALL getLock() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setLock(sal_Bool _bLock) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XWindow
    virtual void SAL_CALL setPosSize( ::sal_Int32 X, ::sal_Int32 Y, ::sal_Int32 Width, ::sal_Int32 Height, ::sal_Int16 Flags ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getPosSize(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setVisible( sal_Bool Visible ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setEnable( sal_Bool Enable ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setFocus(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addPaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removePaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    bool Commit() {return m_pCellControl->Commit();}
    void ImplInitWindow( vcl::Window& rParent, const InitWindowFacet _eInitWhat )
        { m_pCellControl->ImplInitWindow( rParent, _eInitWhat ); }

    bool isAlignedController() const { return m_pCellControl->isAlignedController(); }
    void AlignControl(sal_Int16 nAlignment)
        { m_pCellControl->AlignControl(nAlignment);}

protected:
    virtual vcl::Window* getEventWindow() const;
    virtual void onWindowEvent( const sal_uLong _nEventId, const vcl::Window& _rWindow, const void* _pEventData );

    // default implementations call our focus listeners, don't forget to call them if you override this
    virtual void onFocusGained( const ::com::sun::star::awt::FocusEvent& _rEvent );
    virtual void onFocusLost( const ::com::sun::star::awt::FocusEvent& _rEvent );

private:
    DECL_LINK( OnWindowEvent, VclWindowEvent* );
};


class FmXDataCell : public FmXGridCell
{
public:
    TYPEINFO_OVERRIDE();
    FmXDataCell( DbGridColumn* pColumn, DbCellControl& _rControl )
        :FmXGridCell( pColumn, &_rControl )
    {
    }

    virtual void PaintFieldToCell(OutputDevice& rDev,
               const Rectangle& rRect,
               const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& xField,
               const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);

    void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& xField,
                         const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
                { m_pCellControl->UpdateFromField(xField, xFormatter); }

protected:
    void UpdateFromColumn();
};


class FmXTextCell : public FmXDataCell
{
protected:
    /** determines whether the text of this cell can be painted directly, without
        using the painter control

        If this is <TRUE/>, the <member>PaintCell</member> method will simply use the text as returned
        by <member>GetText</member>, and draw it onto the device passed to <member>PaintFieldToCell</member>,
        while respecting the current alignment settings.

        If this is <FALSE/>, the <member>PaintFieldToCell</member> request will be forwarded to the painter
        control (<member>m_pPainter</member>). This is more expensive, but the only option
        if your painting involves more that a simple DrawText.

        This member is <TRUE/> by default, and can be modified by derived classes.
    */
    bool    m_bFastPaint;

public:
    TYPEINFO_OVERRIDE();
    FmXTextCell( DbGridColumn* pColumn, DbCellControl& _rControl );

    virtual void PaintFieldToCell(OutputDevice& rDev,
               const Rectangle& rRect,
               const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& xField,
               const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) SAL_OVERRIDE;

    OUString GetText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField,
                   const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter,
                   Color** ppColor = NULL)
            {return m_pCellControl->GetFormatText(_rxField, xFormatter, ppColor);}
};


typedef ::cppu::ImplHelper2 <   ::com::sun::star::awt::XTextComponent
                            ,   ::com::sun::star::form::XChangeBroadcaster
                            >   FmXEditCell_Base;
class FmXEditCell : public FmXTextCell,
                    public FmXEditCell_Base
{
private:
    OUString                     m_sValueOnEnter;

protected:
    ::cppu::OInterfaceContainerHelper   m_aTextListeners;
    ::cppu::OInterfaceContainerHelper   m_aChangeListeners;
    ::svt::IEditImplementation*         m_pEditImplementation;
    bool                                m_bOwnEditImplementation;

    virtual ~FmXEditCell();
public:
    FmXEditCell( DbGridColumn* pColumn, DbCellControl& _rControl );

    DECLARE_UNO3_AGG_DEFAULTS(FmXEditCell, FmXTextCell)
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// OComponentHelper
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

// ::com::sun::star::awt::XTextComponent
    virtual void SAL_CALL addTextListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener >& l) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeTextListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener >& l) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setText(const OUString& aText) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL insertText(const ::com::sun::star::awt::Selection& Sel, const OUString& Text) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getText() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getSelectedText() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setSelection(const ::com::sun::star::awt::Selection& aSelection) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::awt::Selection SAL_CALL getSelection() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isEditable() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setEditable(sal_Bool bEditable) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setMaxTextLen(sal_Int16 nLen) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int16 SAL_CALL getMaxTextLen() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XChangeBroadcaster
    virtual void SAL_CALL addChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XChangeListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XChangeListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

protected:
    virtual void onWindowEvent( const sal_uLong _nEventId, const vcl::Window& _rWindow, const void* _pEventData ) SAL_OVERRIDE;

    virtual void onFocusGained( const ::com::sun::star::awt::FocusEvent& _rEvent ) SAL_OVERRIDE;
    virtual void onFocusLost( const ::com::sun::star::awt::FocusEvent& _rEvent ) SAL_OVERRIDE;

private:
    void onTextChanged();
};


typedef ::cppu::ImplHelper2 <   ::com::sun::star::awt::XCheckBox
                            ,   ::com::sun::star::awt::XButton
                            >   FmXCheckBoxCell_Base;
class FmXCheckBoxCell : public FmXDataCell,
                        public FmXCheckBoxCell_Base
{
    ::cppu::OInterfaceContainerHelper   m_aItemListeners;
    ::cppu::OInterfaceContainerHelper   m_aActionListeners;
    OUString                     m_aActionCommand;
    CheckBox*                           m_pBox;

protected:
    virtual ~FmXCheckBoxCell();

public:
    FmXCheckBoxCell( DbGridColumn* pColumn, DbCellControl& _rControl );

// UNO
    DECLARE_UNO3_AGG_DEFAULTS(FmXCheckBoxCell, FmXDataCell)
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// OComponentHelper
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

// ::com::sun::star::awt::XCheckBox
    virtual void SAL_CALL addItemListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeItemListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int16 SAL_CALL getState() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setState(sal_Int16 n) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setLabel(const OUString& Label) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL enableTriState(sal_Bool b) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XButton
    virtual void SAL_CALL addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    //virtual void SAL_CALL setLabel( const OUString& Label ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setActionCommand( const OUString& Command ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

protected:
    virtual vcl::Window* getEventWindow() const SAL_OVERRIDE;
    virtual void onWindowEvent( const sal_uLong _nEventId, const vcl::Window& _rWindow, const void* _pEventData ) SAL_OVERRIDE;
};


typedef ::cppu::ImplHelper1 <   ::com::sun::star::awt::XListBox
                            >   FmXListBoxCell_Base;
class FmXListBoxCell    :public FmXTextCell
                        ,public FmXListBoxCell_Base
{
    ::cppu::OInterfaceContainerHelper   m_aItemListeners,
                                        m_aActionListeners;
    ListBox*                            m_pBox;

protected:
    virtual ~FmXListBoxCell();

public:
    FmXListBoxCell( DbGridColumn* pColumn, DbCellControl& _rControl );

    DECLARE_UNO3_AGG_DEFAULTS(FmXListBoxCell, FmXTextCell)
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// OComponentHelper
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

// ::com::sun::star::awt::XListBox
    virtual void SAL_CALL addItemListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeItemListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addActionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeActionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addItem(const OUString& aItem, sal_Int16 nPos) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addItems(const ::com::sun::star::uno::Sequence< OUString >& aItems, sal_Int16 nPos) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeItems(sal_Int16 nPos, sal_Int16 nCount) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int16 SAL_CALL getItemCount() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getItem(sal_Int16 nPos) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getItems() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int16 SAL_CALL getSelectedItemPos() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< sal_Int16 > SAL_CALL getSelectedItemsPos() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getSelectedItem() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSelectedItems() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL SAL_CALL selectItemPos(sal_Int16 nPos, sal_Bool bSelect) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL SAL_CALL selectItemsPos(const ::com::sun::star::uno::Sequence< sal_Int16 >& aPositions, sal_Bool bSelect) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL SAL_CALL selectItem(const OUString& aItem, sal_Bool bSelect) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isMutipleMode() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL SAL_CALL setMultipleMode(sal_Bool bMulti) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int16 SAL_CALL getDropDownLineCount() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL SAL_CALL setDropDownLineCount(sal_Int16 nLines) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL SAL_CALL makeVisible(sal_Int16 nEntry) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

protected:
    virtual void onWindowEvent( const sal_uLong _nEventId, const vcl::Window& _rWindow, const void* _pEventData ) SAL_OVERRIDE;

    DECL_LINK( OnDoubleClick, void* );
};


typedef ::cppu::ImplHelper1 <   ::com::sun::star::awt::XComboBox
                            >   FmXComboBoxCell_Base;
class FmXComboBoxCell   :public FmXTextCell
                        ,public FmXComboBoxCell_Base
{
private:
    ::cppu::OInterfaceContainerHelper   m_aItemListeners,
                                        m_aActionListeners;
    ComboBox*                           m_pComboBox;

protected:
    virtual ~FmXComboBoxCell();

public:
    FmXComboBoxCell( DbGridColumn* pColumn, DbCellControl& _rControl );

    DECLARE_UNO3_AGG_DEFAULTS(FmXListBoxCell, FmXTextCell)
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // OComponentHelper
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    // XComboBox
    virtual void SAL_CALL addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& _Listener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& _Listener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& _Listener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& _Listener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addItem( const OUString& _Item, ::sal_Int16 _Pos ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addItems( const ::com::sun::star::uno::Sequence< OUString >& _Items, ::sal_Int16 _Pos ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeItems( ::sal_Int16 nPos, ::sal_Int16 nCount ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::sal_Int16 SAL_CALL getItemCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getItem( ::sal_Int16 _Pos ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getItems(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::sal_Int16 SAL_CALL getDropDownLineCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setDropDownLineCount( ::sal_Int16 _Lines ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

protected:
    virtual void onWindowEvent( const sal_uLong _nEventId, const vcl::Window& _rWindow, const void* _pEventData ) SAL_OVERRIDE;
};


typedef ::cppu::ImplHelper2 <   ::com::sun::star::awt::XTextComponent
                            ,   ::com::sun::star::lang::XUnoTunnel
                            >   FmXFilterCell_Base;
class FmXFilterCell :public FmXGridCell
                    ,public FmXFilterCell_Base
{
    ::cppu::OInterfaceContainerHelper m_aTextListeners;
protected:
    virtual ~FmXFilterCell();
public:
    TYPEINFO_OVERRIDE();
    FmXFilterCell(DbGridColumn* pColumn = NULL, DbCellControl* pControl = NULL);


    DECLARE_UNO3_AGG_DEFAULTS(FmXFilterCell, FmXGridCell)
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// helpers for XUnoTunnel
    static const ::com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();

//  painting the filter text
    virtual void PaintCell(OutputDevice& rDev, const Rectangle& rRect);
    void Update(){m_pCellControl->Update();}

// OComponentHelper
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

// ::com::sun::star::awt::XTextComponent
    virtual void SAL_CALL addTextListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener >& l) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeTextListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener >& l) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setText(const OUString& aText) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL insertText(const ::com::sun::star::awt::Selection& Sel, const OUString& Text) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getText() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getSelectedText() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setSelection(const ::com::sun::star::awt::Selection& aSelection) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::awt::Selection SAL_CALL getSelection() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isEditable() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setEditable(sal_Bool bEditable) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setMaxTextLen(sal_Int16 nLen) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int16 SAL_CALL getMaxTextLen() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

protected:
    DECL_LINK( OnCommit, void* );
};

#endif // INCLUDED_SVX_SOURCE_INC_GRIDCELL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
