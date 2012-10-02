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

#ifndef _SVX_GRIDCELL_HXX
#define _SVX_GRIDCELL_HXX

#include <svx/gridctrl.hxx>

#include "sqlparserclient.hxx"
#include "typeconversionclient.hxx"

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
#include <comphelper/componentcontext.hxx>
#include <cppuhelper/component.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <tools/diagnose_ex.h>
#include <tools/rtti.hxx>

class DbCellControl;
class Edit;
class FmXGridCell;

//==================================================================
// FmMutexHelper
//==================================================================
class FmMutexHelper
{
protected:
    ::osl::Mutex    m_aMutex;
};

//==================================================================
// DbGridColumn, Spaltenbeschreibung
//==================================================================
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
    sal_Bool                m_bReadOnly : 1;
    sal_Bool                m_bAutoValue : 1;
    sal_Bool                m_bInSave : 1;
    sal_Bool                m_bNumeric : 1;
    sal_Bool                m_bObject : 1;  // Verweist die Column auf ein Object Datentyp?
    sal_Bool                m_bHidden : 1;
    sal_Bool                m_bLocked : 1;
    sal_Bool                m_bDateTime : 1;

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
        ,m_bReadOnly(sal_False)
        ,m_bAutoValue(sal_False)
        ,m_bInSave(sal_False)
        ,m_bNumeric(sal_False)
        ,m_bObject(sal_False)
        ,m_bHidden(sal_False)
        ,m_bLocked(sal_False)
        ,m_bDateTime(sal_False)
    {
    }

    ~DbGridColumn();

    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& getModel() const { return m_xModel; }
    void  setModel(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  _xModel);


    sal_uInt16  GetId() const {return m_nId;}
    sal_Bool    IsReadOnly() const {return m_bReadOnly;}
    sal_Bool    IsAutoValue() const {return m_bAutoValue;}
    sal_Bool    IsUpdating() const {return m_bInSave;}
    sal_Int16   GetAlignment() const {return m_nAlign;}
    sal_Int16   GetType() const {return m_nFieldType;}
    sal_Int16   GetFieldPos() const {return m_nFieldPos; }
    sal_Bool    IsNumeric() const {return m_bNumeric;}
    sal_Bool    IsDateTime() const {return m_bDateTime;}
    sal_Bool    IsObject() const {return m_bObject;}
    sal_Bool    IsHidden() const {return m_bHidden;}
    sal_Int32   GetKey() const {return m_nFormatKey;}
    const   ::svt::CellControllerRef& GetController() const {return m_bLocked ? s_xEmptyController : m_xController;}
    const   ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& GetField() const {return m_xField;}
    DbGridControl& GetParent() const {return m_rParent;}
    FmXGridCell* GetCell() const {return m_pCell;}

    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >  GetCurrentFieldValue() const;

    //      Zeichnen eines Feldes an einer Position, ist ein ::com::sun::star::sdbcx::View gesetzt
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
    sal_Bool    Commit();

    //      freigeben aller Daten, die fuer den AliveMode noetig sind
    void    Clear();

    OUString  GetCellText(const DbGridRow* pRow, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) const;
    OUString  GetCellText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& xField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) const;

    void    SetReadOnly(sal_Bool bRead){m_bReadOnly = bRead;}
    void    SetObject(sal_Int16 nPos) {m_bObject = m_bReadOnly = sal_True; m_nFieldPos = nPos;}

    void    ImplInitWindow( Window& rParent, const InitWindowFacet _eInitWhat );

    // Properties, die auf den ::com::sun::star::frame::Controller durchschlagen koennen
    sal_Int16   SetAlignment(sal_Int16 _nAlign);
        // if _nAlign is -1, the alignment is calculated from the type of the field we are bound to
        // the value really set is returned
    sal_Int16   SetAlignmentFromModel(sal_Int16 nStandardAlign);
        // set the alignment according to the "Align"-property of m_xModel, use the given standard
        // alignment if the property if void, return the really set alignment

    // column locking
    sal_Bool    isLocked() const { return m_bLocked; }
    void    setLock(sal_Bool _bLock);

private:
    /** attaches or detaches our cell object to the SctriptEventAttacherManager implemented
        by our model's parent
    */
    void    impl_toggleScriptManager_nothrow( bool _bAttach );
};

//==================================================================
// DbCellControl, liefert die Daten fuer einen CellController
// wird in der Regel nur für komplexe Controls wie z.B ComboBoxen
// benoetigt
//==================================================================
class DbCellControl
        :public ::svxform::OTypeConversionClient
        ,public ::svxform::OStaticDataAccessTools
        ,public FmMutexHelper           // _before_ the listener, so the listener is to be destroyed first!
        ,public ::comphelper::OPropertyChangeListener
{
private:
    ::comphelper::OPropertyChangeMultiplexer*   m_pModelChangeBroadcaster;
    ::comphelper::OPropertyChangeMultiplexer*   m_pFieldChangeBroadcaster;

private:
    sal_Bool                    m_bTransparent : 1;
    sal_Bool                    m_bAlignedController : 1;
    sal_Bool                    m_bAccessingValueProperty : 1;

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >
                                m_xCursor;

protected:
    DbGridColumn&               m_rColumn;
    Window*                     m_pPainter;
    Window*                     m_pWindow;

protected:
    // attribute access
    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& getCursor() const { return m_xCursor; }

    // control transparency
    inline  sal_Bool    isTransparent( ) const { return m_bTransparent; }
    inline  void        setTransparent( sal_Bool _bSet ) { m_bTransparent = _bSet; }

    // control alignment
    inline  void        setAlignedController( sal_Bool _bAlign = sal_True ) { m_bAlignedController = _bAlign; }


    /** determined whether or not the value property is locked
    @see lockValueProperty
    */
    inline  sal_Bool    isValuePropertyLocked() const;

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
    DbCellControl(DbGridColumn& _rColumn, sal_Bool _bText = sal_True);
    virtual ~DbCellControl();


    Window& GetWindow() const
    {
        ENSURE_OR_THROW( m_pWindow, "no window" );
        return *m_pWindow;
    }

    // control alignment
    inline  sal_Bool    isAlignedController() const { return m_bAlignedController; }
            void        AlignControl(sal_Int16 nAlignment);

    void SetTextLineColor();
    void SetTextLineColor(const Color& _rColor);

    // Initialisieren bevor ein Control angezeigt wird
    virtual void Init( Window& rParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor );
    virtual ::svt::CellControllerRef CreateController() const = 0;

    // Schreiben des Wertes in das Model
    sal_Bool Commit();

    // Formatting the field data to output text
    virtual OUString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL) = 0;

    virtual void Update(){}
    // Refresh the control by the field data
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) = 0;

    // Painten eines Zellinhalts im vorgegeben Rechteck
    virtual void PaintFieldToCell( OutputDevice& rDev, const Rectangle& rRect, const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);
    virtual void PaintCell( OutputDevice& _rDev, const Rectangle& _rRect );

    void  ImplInitWindow( Window& rParent, const InitWindowFacet _eInitWhat );

    double GetValue(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) const;

protected:
    void    invalidatedController();

    /** commits the content of the control (e.g. the text of an edit field) into the column model
        (e.g. the "Text" property of the model).
        <p>To be overwritten in derived classes.</p>
    @see updateFromModel
    */
    virtual sal_Bool commitControl( ) = 0;

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
    virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException);

private:
    void implDoPropertyListening( const OUString& _rPropertyName, sal_Bool _bWarnIfNotExistent = sal_True );

    /// updates the "readonly" setting on m_pWindow, according to the respective property value in the given model
    void implAdjustReadOnly( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel,bool i_bReadOnly );

    /// updates the "enabled" setting on m_pWindow, according to the respective property value in the given model
    void implAdjustEnabled( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel );
};

//==================================================================
//------------------------------------------------------------------
inline  sal_Bool    DbCellControl::isValuePropertyLocked() const
{
    return m_bAccessingValueProperty;
}

//------------------------------------------------------------------
inline  void        DbCellControl::lockValueProperty()
{
    OSL_ENSURE( !isValuePropertyLocked(), "DbCellControl::lockValueProperty: not to be nested!" );
    m_bAccessingValueProperty = sal_True;
}

//------------------------------------------------------------------
inline  void        DbCellControl::unlockValueProperty()
{
    OSL_ENSURE( isValuePropertyLocked(), "DbCellControl::lockValueProperty: not locked so far!" );
    m_bAccessingValueProperty = sal_False;
}

//==================================================================
/** a field which is bound to a column which supports the MaxTextLen property
*/
class DbLimitedLengthField : public DbCellControl
{
public:
    TYPEINFO();

protected:
    DbLimitedLengthField( DbGridColumn& _rColumn );

protected:
    // DbCellControl
    virtual void implAdjustGenericFieldSetting( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel );

protected:
    inline void implSetMaxTextLen( sal_Int16 _nMaxLen )
    {
        implSetEffectiveMaxTextLen( _nMaxLen ? _nMaxLen : EDIT_NOLIMIT );
    }
    virtual void implSetEffectiveMaxTextLen( sal_Int16 _nMaxLen );
};

//==================================================================
class DbTextField : public DbLimitedLengthField
{
    ::svt::IEditImplementation* m_pEdit;
    ::svt::IEditImplementation* m_pPainterImplementation;
    sal_Int16                   m_nKeyType;
    sal_Bool                    m_bIsSimpleEdit;

protected:
    ~DbTextField( );

public:
    TYPEINFO();
    DbTextField(DbGridColumn& _rColumn);

    ::svt::IEditImplementation* GetEditImplementation() { return m_pEdit; }
    sal_Bool                    IsSimpleEdit() const { return m_bIsSimpleEdit; }

    virtual void Init( Window& rParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor );
    virtual OUString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);
    virtual ::svt::CellControllerRef CreateController() const;
    virtual void PaintFieldToCell( OutputDevice& _rDev, const Rectangle& _rRect,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter );

protected:
    // DbCellControl
    virtual sal_Bool    commitControl( );
    virtual void        updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel );
    // DbLimitedLengthField
    virtual void        implSetEffectiveMaxTextLen( sal_Int16 _nMaxLen );
};

//==================================================================
class DbFormattedField : public DbLimitedLengthField
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >  m_xSupplier;
    sal_Int16                       m_nKeyType;


public:
    TYPEINFO();
    DbFormattedField(DbGridColumn& _rColumn);
    virtual ~DbFormattedField();


    virtual void Init( Window& rParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor );
    virtual OUString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);
    virtual ::svt::CellControllerRef CreateController() const;

protected:
    // DbCellControl
    virtual sal_Bool    commitControl( );
    virtual void        updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel );

    // OPropertyChangeListener
    virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException);
};

//==================================================================
class DbCheckBox : public DbCellControl
{
public:
    TYPEINFO();
    DbCheckBox(DbGridColumn& _rColumn);

    virtual void Init( Window& rParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor );
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);
    virtual ::svt::CellControllerRef CreateController() const;
    virtual void PaintFieldToCell(OutputDevice& rDev, const Rectangle& rRect,
                          const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField,
                          const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);
    virtual OUString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);

protected:
    // DbCellControl
    virtual sal_Bool    commitControl( );
    virtual void        updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel );
};

//==================================================================
class DbComboBox : public DbCellControl
{
    sal_Int16         m_nKeyType;

public:
    TYPEINFO();
    DbComboBox(DbGridColumn& _rColumn);

    virtual void Init( Window& rParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor );
    virtual OUString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);
    virtual ::svt::CellControllerRef CreateController() const;

    void SetList(const ::com::sun::star::uno::Any& rItems);

protected:
    // DbCellControl
    virtual sal_Bool    commitControl( );
    virtual void        updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel );

    virtual void        implAdjustGenericFieldSetting( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel );

    // OPropertyChangeListener
    virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException);
};

//==================================================================
class DbListBox     :public DbCellControl
{
    sal_Bool              m_bBound  : 1;
    ::com::sun::star::uno::Sequence< OUString > m_aValueList;

public:
    TYPEINFO();
    DbListBox(DbGridColumn& _rColumn);

    virtual void Init( Window& rParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor );
    virtual OUString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);
    virtual ::svt::CellControllerRef CreateController() const;

    void SetList(const ::com::sun::star::uno::Any& rItems);

protected:
    // DbCellControl
    virtual sal_Bool    commitControl( );
    virtual void        updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel );

    virtual void        implAdjustGenericFieldSetting( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel );

    // OPropertyChangeListener
    virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException);
};

//==================================================================
class DbPatternField : public DbCellControl
{
public:
    TYPEINFO();
    DbPatternField( DbGridColumn& _rColumn, const ::comphelper::ComponentContext& _rContext );
    virtual void Init( Window& rParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor );
    virtual OUString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);
    virtual ::svt::CellControllerRef CreateController() const;

protected:
    /// DbCellControl
    virtual sal_Bool    commitControl( );
    virtual void        updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel );

    virtual void        implAdjustGenericFieldSetting( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel );

private:
    OUString  impl_formatText(const OUString& _rText);

private:
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr< ::dbtools::FormattedColumnValue >  m_pValueFormatter;
    ::std::auto_ptr< ::dbtools::FormattedColumnValue >  m_pPaintFormatter;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    ::comphelper::ComponentContext                      m_aContext;
};

//==================================================================
class DbSpinField : public DbCellControl
{
private:
    sal_Int16   m_nStandardAlign;

public:
    TYPEINFO();

protected:
    DbSpinField( DbGridColumn& _rColumn, sal_Int16 _nStandardAlign = com::sun::star::awt::TextAlign::RIGHT );

public:
    virtual void                        Init( Window& rParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& _rxCursor );
    virtual ::svt::CellControllerRef    CreateController() const;

protected:
    virtual SpinField*  createField(
                            Window* _pParent,
                            WinBits _nFieldStyle,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel
                        ) = 0;
};

//==================================================================
class DbDateField : public DbSpinField
{
public:
    TYPEINFO();
    DbDateField(DbGridColumn& _rColumn);
    virtual OUString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);

protected:
    // DbCellControl
    virtual sal_Bool    commitControl( );
    virtual void        updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel );

    // DbSpinField
    virtual SpinField*  createField(
                            Window* _pParent,
                            WinBits _nFieldStyle,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel
                        );

    /// initializes everything which relates to the properties describing the numeric behaviour
    virtual void    implAdjustGenericFieldSetting( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel );
};

//==================================================================
class DbTimeField : public DbSpinField
{
public:
    TYPEINFO();
    DbTimeField(DbGridColumn& _rColumn);
    virtual OUString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);

protected:
    // DbCellControl
    virtual sal_Bool    commitControl( );
    virtual void        updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel );

    // DbSpinField
    virtual SpinField*  createField(
                            Window* _pParent,
                            WinBits _nFieldStyle,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel
                        );

    /// initializes everything which relates to the properties describing the numeric behaviour
    virtual void    implAdjustGenericFieldSetting( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel );
};

//==================================================================
class DbCurrencyField : public DbSpinField
{
    sal_Int16  m_nScale;

public:
    TYPEINFO();
    DbCurrencyField(DbGridColumn& _rColumn);
    virtual OUString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);

    double GetCurrency(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) const;

protected:
    // DbCellControl
    virtual sal_Bool    commitControl( );
    virtual void        updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel );

    // DbSpinField
    virtual SpinField*  createField(
                            Window* _pParent,
                            WinBits _nFieldStyle,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel
                        );

    /// initializes everything which relates to the properties describing the numeric behaviour
    virtual void    implAdjustGenericFieldSetting( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel );
};

//==================================================================
class DbNumericField : public DbSpinField
{
public:
    TYPEINFO();
    DbNumericField(DbGridColumn& _rColumn);
    virtual OUString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);

protected:
    // DbCellControl
    virtual sal_Bool    commitControl( );
    virtual void        updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel );

    // DbSpinField
    virtual SpinField*  createField(
                            Window* _pParent,
                            WinBits _nFieldStyle,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel
                        );

    /// initializes everything which relates to the properties describing the numeric behaviour
    void    implAdjustGenericFieldSetting( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel );
};

//==================================================================
class DbFilterField
        :public DbCellControl
        ,public ::svxform::OSQLParserClient
{
    ::com::sun::star::uno::Sequence< OUString >  m_aValueList;
    OUString   m_aText;
    Link    m_aCommitLink;
    sal_Int16   m_nControlClass;
    sal_Bool    m_bFilterList : 1;
    sal_Bool    m_bFilterListFilled : 1;
    sal_Bool    m_bBound : 1;

public:
    TYPEINFO();
    DbFilterField(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,DbGridColumn& _rColumn);
    virtual ~DbFilterField();

    virtual void Init( Window& rParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor );
    virtual ::svt::CellControllerRef CreateController() const;
    virtual void PaintCell(OutputDevice& rDev, const Rectangle& rRect);
    virtual void Update();
    virtual OUString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);

    const OUString& GetText() const {return m_aText;}
    void SetText(const OUString& rText);

    void SetCommitHdl( const Link& rLink ) { m_aCommitLink = rLink; }
    const Link& GetCommitHdl() const { return m_aCommitLink; }

protected:

    // DbCellControl
    virtual sal_Bool    commitControl( );
    virtual void        updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel );

protected:
    void SetList(const ::com::sun::star::uno::Any& rItems, sal_Bool bComboBox);
    void CreateControl(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xModel);
    DECL_LINK( OnClick, void* );
};

//==================================================================
// Base class providing the access to a grid cell
//==================================================================
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

    DECLARE_UNO3_AGG_DEFAULTS(FmXGridCell, OComponentHelper);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException);

    void SetTextLineColor();
    void SetTextLineColor(const Color& _rColor);

// XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

// OComponentHelper
    virtual void SAL_CALL disposing();

// ::com::sun::star::lang::XComponent
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException){OComponentHelper::dispose();}
    virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener)throw(::com::sun::star::uno::RuntimeException)       { OComponentHelper::addEventListener(aListener);}
    virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener)throw(::com::sun::star::uno::RuntimeException)        { OComponentHelper::removeEventListener(aListener);}

// ::com::sun::star::awt::XControl
    virtual void SAL_CALL setContext(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& /*Context*/) throw(::com::sun::star::uno::RuntimeException){}
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL getContext() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL createPeer(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& /*Toolkit*/, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& /*Parent*/) throw(::com::sun::star::uno::RuntimeException){}

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > SAL_CALL getPeer() throw (::com::sun::star::uno::RuntimeException) {return ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > ();}
    virtual sal_Bool SAL_CALL setModel(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& /*Model*/) throw (::com::sun::star::uno::RuntimeException) {return sal_False;}
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > SAL_CALL getModel() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XView > SAL_CALL getView() throw (::com::sun::star::uno::RuntimeException) {return ::com::sun::star::uno::Reference< ::com::sun::star::awt::XView > ();}
    virtual void SAL_CALL setDesignMode(sal_Bool /*bOn*/) throw (::com::sun::star::uno::RuntimeException) {}
    virtual sal_Bool SAL_CALL isDesignMode() throw (::com::sun::star::uno::RuntimeException) {return sal_False;}
    virtual sal_Bool SAL_CALL isTransparent() throw (::com::sun::star::uno::RuntimeException) {return sal_False;}

// ::com::sun::star::form::XBoundControl
    virtual sal_Bool SAL_CALL getLock() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setLock(sal_Bool _bLock) throw(::com::sun::star::uno::RuntimeException);

    // XWindow
    virtual void SAL_CALL setPosSize( ::sal_Int32 X, ::sal_Int32 Y, ::sal_Int32 Width, ::sal_Int32 Height, ::sal_Int16 Flags ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getPosSize(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setVisible( ::sal_Bool Visible ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setEnable( ::sal_Bool Enable ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setFocus(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

    sal_Bool Commit() {return m_pCellControl->Commit();}
    void ImplInitWindow( Window& rParent, const InitWindowFacet _eInitWhat )
        { m_pCellControl->ImplInitWindow( rParent, _eInitWhat ); }

    sal_Bool isAlignedController() const { return m_pCellControl->isAlignedController(); }
    void AlignControl(sal_Int16 nAlignment)
        { m_pCellControl->AlignControl(nAlignment);}

protected:
    virtual Window* getEventWindow() const;
    virtual void onWindowEvent( const sal_uLong _nEventId, const Window& _rWindow, const void* _pEventData );

    // default implementations call our focus listeners, don't forget to call them if you override this
    virtual void onFocusGained( const ::com::sun::star::awt::FocusEvent& _rEvent );
    virtual void onFocusLost( const ::com::sun::star::awt::FocusEvent& _rEvent );

private:
    DECL_LINK( OnWindowEvent, VclWindowEvent* );
};

//==================================================================
class FmXDataCell : public FmXGridCell
{
public:
    TYPEINFO();
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

//==================================================================
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
    sal_Bool    m_bFastPaint;

public:
    TYPEINFO();
    FmXTextCell( DbGridColumn* pColumn, DbCellControl& _rControl );

    virtual void PaintFieldToCell(OutputDevice& rDev,
               const Rectangle& rRect,
               const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& xField,
               const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);

    OUString GetText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxField,
                   const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter,
                   Color** ppColor = NULL)
            {return m_pCellControl->GetFormatText(_rxField, xFormatter, ppColor);}
};

//==================================================================
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

    DECLARE_UNO3_AGG_DEFAULTS(FmXEditCell, FmXTextCell);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException);

// XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

// OComponentHelper
    virtual void SAL_CALL disposing();

// ::com::sun::star::awt::XTextComponent
    virtual void SAL_CALL addTextListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener >& l) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTextListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener >& l) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setText(const OUString& aText) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertText(const ::com::sun::star::awt::Selection& Sel, const OUString& Text) throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getText() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getSelectedText() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setSelection(const ::com::sun::star::awt::Selection& aSelection) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Selection SAL_CALL getSelection() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isEditable() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setEditable(sal_Bool bEditable) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setMaxTextLen(sal_Int16 nLen) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getMaxTextLen() throw(::com::sun::star::uno::RuntimeException);

    // XChangeBroadcaster
    virtual void SAL_CALL addChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XChangeListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XChangeListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

protected:
    virtual void onWindowEvent( const sal_uLong _nEventId, const Window& _rWindow, const void* _pEventData );

    virtual void onFocusGained( const ::com::sun::star::awt::FocusEvent& _rEvent );
    virtual void onFocusLost( const ::com::sun::star::awt::FocusEvent& _rEvent );

private:
    void onTextChanged();
};

//==================================================================
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
    DECLARE_UNO3_AGG_DEFAULTS(FmXCheckBoxCell, FmXDataCell);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

// OComponentHelper
    virtual void SAL_CALL disposing();

// ::com::sun::star::awt::XCheckBox
    virtual void SAL_CALL addItemListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeItemListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getState() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setState(sal_Int16 n) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setLabel(const OUString& Label) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL enableTriState(sal_Bool b) throw(::com::sun::star::uno::RuntimeException);

    // XButton
    virtual void SAL_CALL addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l ) throw (::com::sun::star::uno::RuntimeException);
    //virtual void SAL_CALL setLabel( const OUString& Label ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setActionCommand( const OUString& Command ) throw (::com::sun::star::uno::RuntimeException);

protected:
    virtual Window* getEventWindow() const;
    virtual void onWindowEvent( const sal_uLong _nEventId, const Window& _rWindow, const void* _pEventData );
};

//==================================================================
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

    DECLARE_UNO3_AGG_DEFAULTS(FmXListBoxCell, FmXTextCell);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

// OComponentHelper
    virtual void SAL_CALL disposing();

// ::com::sun::star::awt::XListBox
    virtual void SAL_CALL addItemListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeItemListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addActionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeActionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addItem(const OUString& aItem, sal_Int16 nPos) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addItems(const ::com::sun::star::uno::Sequence< OUString >& aItems, sal_Int16 nPos) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeItems(sal_Int16 nPos, sal_Int16 nCount) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getItemCount() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getItem(sal_Int16 nPos) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getItems() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getSelectedItemPos() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int16 > SAL_CALL getSelectedItemsPos() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getSelectedItem() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSelectedItems() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL SAL_CALL selectItemPos(sal_Int16 nPos, sal_Bool bSelect) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL SAL_CALL selectItemsPos(const ::com::sun::star::uno::Sequence< sal_Int16 >& aPositions, sal_Bool bSelect) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL SAL_CALL selectItem(const OUString& aItem, sal_Bool bSelect) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isMutipleMode() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL SAL_CALL setMultipleMode(sal_Bool bMulti) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getDropDownLineCount() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL SAL_CALL setDropDownLineCount(sal_Int16 nLines) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL SAL_CALL makeVisible(sal_Int16 nEntry) throw(::com::sun::star::uno::RuntimeException);

protected:
    virtual void onWindowEvent( const sal_uLong _nEventId, const Window& _rWindow, const void* _pEventData );

    DECL_LINK( OnDoubleClick, void* );
};

//==================================================================
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

    DECLARE_UNO3_AGG_DEFAULTS(FmXListBoxCell, FmXTextCell);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // OComponentHelper
    virtual void SAL_CALL disposing();

    // XComboBox
    virtual void SAL_CALL addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& _Listener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& _Listener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& _Listener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& _Listener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addItem( const OUString& _Item, ::sal_Int16 _Pos ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addItems( const ::com::sun::star::uno::Sequence< OUString >& _Items, ::sal_Int16 _Pos ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeItems( ::sal_Int16 nPos, ::sal_Int16 nCount ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int16 SAL_CALL getItemCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getItem( ::sal_Int16 _Pos ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getItems(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int16 SAL_CALL getDropDownLineCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDropDownLineCount( ::sal_Int16 _Lines ) throw (::com::sun::star::uno::RuntimeException);

protected:
    virtual void onWindowEvent( const sal_uLong _nEventId, const Window& _rWindow, const void* _pEventData );
};

//==================================================================
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
    TYPEINFO();
    FmXFilterCell(DbGridColumn* pColumn = NULL, DbCellControl* pControl = NULL);


    DECLARE_UNO3_AGG_DEFAULTS(FmXFilterCell, FmXGridCell);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

// XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

// helpers for XUnoTunnel
    static const ::com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();

//  painting the filter text
    virtual void PaintCell(OutputDevice& rDev, const Rectangle& rRect);
    void Update(){m_pCellControl->Update();}

// OComponentHelper
    virtual void SAL_CALL disposing();

// ::com::sun::star::awt::XTextComponent
    virtual void SAL_CALL addTextListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener >& l) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTextListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener >& l) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setText(const OUString& aText) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertText(const ::com::sun::star::awt::Selection& Sel, const OUString& Text) throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getText() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getSelectedText() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setSelection(const ::com::sun::star::awt::Selection& aSelection) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Selection SAL_CALL getSelection() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isEditable() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setEditable(sal_Bool bEditable) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setMaxTextLen(sal_Int16 nLen) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getMaxTextLen() throw(::com::sun::star::uno::RuntimeException);

protected:
    DECL_LINK( OnCommit, void* );
};

#endif // _SVX_GRIDCELL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
