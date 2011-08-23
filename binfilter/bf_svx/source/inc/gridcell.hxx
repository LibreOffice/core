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

#ifndef _SVX_GRIDCTRL_HXX
#include "gridctrl.hxx"
#endif

#ifndef _SVX_FMTOOLS_HXX
#include "fmtools.hxx"
#endif


#ifndef _COM_SUN_STAR_FORM_XBOUNDCONTROL_HPP_
#include <com/sun/star/form/XBoundControl.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTEXTCOMPONENT_HPP_
#include <com/sun/star/awt/XTextComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XLISTBOX_HPP_
#include <com/sun/star/awt/XListBox.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_TEXTALIGN_HPP_
#include <com/sun/star/awt/TextAlign.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCHECKBOX_HPP_
#include <com/sun/star/awt/XCheckBox.hpp>
#endif
#ifndef SVX_SQLPARSERCLIENT_HXX
#include "sqlparserclient.hxx"
#endif
class Edit;
namespace binfilter {

class DbCellControl;
class FmXGridCell;

//==================================================================
// FmMutexHelper
//==================================================================
class FmMutexHelper
{
protected:
    ::osl::Mutex	m_aMutex;
};

//==================================================================
// DbGridColumn, Spaltenbeschreibung
//==================================================================
class DbGridColumn
{
    friend class DbGridControl;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >		m_xModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >		m_xField;		// Verbindung zum Datenbankfeld
    CellControllerRef m_xController; // Struktur zum Verwalten der Controls fuer eine Spalte
                                        // diese wird von der DbBrowseBox auf die jeweiligen Zellen
                                        // einer Spalte positioniert
    FmXGridCell*				m_pCell;

protected:
    DbGridControl&		m_rParent;

private:
    sal_Int32				m_nLastVisibleWidth;	// nur gueltig, wenn m_bHidden == sal_True
    sal_Int32				m_nFormatKey;
    sal_Int16				m_nFieldType;
    sal_Int16				m_nTypeId;
    sal_uInt16				m_nId;
    sal_Int16				m_nFieldPos;
    sal_Int16				m_nAlign;						// wird mit TXT_ALIGN_LEFT .... angegeben
    sal_Bool				m_bRequired : 1;
    sal_Bool				m_bReadOnly : 1;
    sal_Bool				m_bAutoValue : 1;
    sal_Bool				m_bInSave : 1;
    sal_Bool				m_bNumeric : 1;
    sal_Bool				m_bObject : 1;	// Verweist die Column auf ein Object Datentyp?
    sal_Bool				m_bHidden : 1;
    sal_Bool				m_bLocked : 1;
    sal_Bool				m_bDateTime : 1;

    static CellControllerRef s_xEmptyController;
        // used by locked columns
public:
    DbGridColumn(sal_uInt16 _nId, DbGridControl& rParent)
        :m_nId(_nId)
        ,m_rParent(rParent)
        ,m_nFieldType(0)
        ,m_nFormatKey(0)
        ,m_pCell(NULL)
        ,m_bNumeric(sal_False)
        ,m_bDateTime(sal_False)
        ,m_nAlign(::com::sun::star::awt::TextAlign::LEFT)
        ,m_nFieldPos(-1)
        ,m_bRequired(sal_False)
        ,m_bReadOnly(sal_False)
        ,m_bAutoValue(sal_False)
        ,m_bInSave(sal_False)
        ,m_bHidden(sal_False)
        ,m_nLastVisibleWidth(-1)
        ,m_nTypeId(0)
        ,m_bLocked(sal_False)
    {
    }

    ~DbGridColumn();

    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& getModel() const { return m_xModel; }
    void  setModel(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  _xModel) { m_xModel = _xModel; }


    sal_uInt16	GetId() const {return m_nId;}
    sal_Bool	IsRequired() const {return m_bRequired;}
    sal_Bool	IsReadOnly() const {return m_bReadOnly;}
    sal_Bool	IsAutoValue() const {return m_bAutoValue;}
    sal_Bool	IsUpdating() const {return m_bInSave;}
    sal_Int16	GetAlignment() const {return m_nAlign;}
    sal_Int16	GetType() const {return m_nFieldType;}
    sal_Int16	GetFieldPos() const {return m_nFieldPos; }
    sal_Bool	IsNumeric() const {return m_bNumeric;}
    sal_Bool	IsDateTime() const {return m_bDateTime;}	
    sal_Bool	IsObject() const {return m_bObject;}
    sal_Bool	IsHidden() const {return m_bHidden;}
    sal_Int32	GetKey() const {return m_nFormatKey;}
    const	CellControllerRef& GetController() const {return m_bLocked ? s_xEmptyController : m_xController;}
    const	::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& GetField() const {return m_xField;}
    DbGridControl& GetParent() const {return m_rParent;}
    FmXGridCell* GetCell() const {return m_pCell;}

    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >	GetCurrentFieldValue() const;

    //		Zeichnen eines Feldes an einer Position, ist ein ::com::sun::star::sdbcx::View gesetzt
    //		uebernimmt dieser das Zeichnen, z.B. fuer CheckBoxen
    void	Paint(OutputDevice& rDev,
                  const Rectangle& rRect,
                  const DbGridRow* pRow,
                  const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);


    //		Inititialierung im alive mode
    //		Ist kein ColumnController gesetzt, wird eine DefaultInitialisierung
    //		vorgenommen
    void	CreateControl(sal_Int32 _nFieldPos, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xField, sal_Int32 nTypeId);
    void	UpdateControl()
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xField(m_xField);
                CreateControl(m_nFieldPos, xField, m_nTypeId);
            }

    //		Editieren einer Zelle
    void	UpdateFromField(const DbGridRow* pRow, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);
    sal_Bool	Commit();

    //		freigeben aller Daten, die fuer den AliveMode noetig sind
    void	Clear();

    XubString  GetCellText(const DbGridRow* pRow, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) const;
    XubString  GetCellText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& xField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) const;

    void	SetReadOnly(sal_Bool bRead){m_bReadOnly = bRead;}
    void	SetObject(sal_Int16 nPos) {m_bObject = m_bReadOnly = sal_True; m_nFieldPos = nPos;}
    void	ImplInitSettings(Window* pParent, sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground);

    // Properties, die auf den ::com::sun::star::frame::Controller durchschlagen koennen
    sal_Int16	SetAlignment(sal_Int16 _nAlign);
        // if _nAlign is -1, the alignment is calculated from the type of the field we are bound to
        // the value really set is returned
    sal_Int16	SetAlignmentFromModel(sal_Int16 nStandardAlign);
        // set the alignment according to the "Align"-property of m_xModel, use the given standard
        // alignment if the property if void, return the really set alignment

    // column locking
    sal_Bool	isLocked() const { return m_bLocked; }
    void	setLock(sal_Bool _bLock);
};

//==================================================================
// DbCellControl, liefert die Daten fuer einen CellController
// wird in der Regel nur für komplexe Controls wie z.B ComboBoxen
// benoetigt
//==================================================================
class DbCellControl
:public ::binfilter::svxform::OStaticDataAccessTools//STRIP001 		,public ::svxform::OStaticDataAccessTools
        ,public FmMutexHelper			// _before_ the listener, so the listener is to be destroyed first!
        ,public ::comphelper::OPropertyChangeListener
{
private:
    ::comphelper::OPropertyChangeMultiplexer*	m_pModelChangeBroadcaster;

private:
    sal_Bool					m_bTransparent : 1;
    sal_Bool					m_bAlignedController : 1;
    sal_Bool					m_bAccessingValueProperty : 1;

protected:
    DbGridColumn&				m_rColumn;
    Window* 					m_pPainter;
    Window* 					m_pWindow;

protected:
    // control transparency
    inline	sal_Bool	isTransparent( ) const { return m_bTransparent; }
    inline	void		setTransparent( sal_Bool _bSet ) { m_bTransparent = _bSet; }

    // control alignment
    inline	void		setAlignedController( sal_Bool _bAlign = sal_True ) { m_bAlignedController = _bAlign; }


    /** determined whether or not the value property is locked
    @see lockValueProperty
    */
    inline	sal_Bool	isValuePropertyLocked() const;

    /** locks the listening at the value property.
        <p>This means that every subsequent change now done on the value property of the model ("Text", or "Value",
        or whatever) is then ignored.<br/>
        This base class uses this setting in <method>Commit</method>.</p>
    @precond
        Value locking can't be nested
    @see unlockValueProperty
    */
    inline	void		lockValueProperty();
    /** unlocks the listening at the value property
    @see lockValueProperty
    */
    inline	void		unlockValueProperty();

protected:
    // adds the given property to the list of properties which we listen for
    void	doPropertyListening( const ::rtl::OUString& _rPropertyName );

    // called whenever a property which affects field settings in general is called
    // you should overwrite this method for every property you add yourself as listener to
    // with doPropertyListening
    virtual void	implAdjustGenericFieldSetting( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel );

    // called by _propertyChanged if a property which denotes the column value has changed
            void	implValuePropertyChanged( );


public:
    DbCellControl(DbGridColumn& _rColumn, sal_Bool _bText = sal_True);
    virtual ~DbCellControl();		 

    
    Window* GetControl() const { return m_pWindow; }

    // control alignment
    inline	sal_Bool	isAlignedController() const { return m_bAlignedController; }
            void		AlignControl(sal_Int16 nAlignment);

    void SetTextLineColor();
    void SetTextLineColor(const Color& _rColor);

    // Initialisieren bevor ein Control angezeigt wird
    virtual void Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor);
    virtual CellControllerRef CreateController() const = 0;

    // Schreiben des Wertes in das Model
    sal_Bool Commit();

    // Formatting the field data to output text
    virtual XubString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL) { return XubString(); }

    virtual void Update(){}
    // Refresh the control by the field data
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) {}

    // Painten eines Zellinhalts im vorgegeben Rechteck
    virtual void Paint(OutputDevice& rDev, const Rectangle& rRect);
    virtual void Paint(OutputDevice& rDev, const Rectangle& rRect, const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter)
        { Paint(rDev, rRect); }

    void  ImplInitSettings(Window* pParent, sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground);	

protected:	
    double GetValue(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) const;

    void	invalidatedController();

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
    virtual void	updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel ) = 0;

protected:
// OPropertyChangeListener
    virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException);

private:
    void implDoPropertyListening( const ::rtl::OUString& _rPropertyName, sal_Bool _bWarnIfNotExistent = sal_True );

    /// updates the "readonly" setting on m_pWindow, according to the respective property value in the given model
    void implAdjustReadOnly( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel );

    /// updates the "enabled" setting on m_pWindow, according to the respective property value in the given model
    void implAdjustEnabled( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel );
};

//==================================================================
//------------------------------------------------------------------
inline	sal_Bool	DbCellControl::isValuePropertyLocked() const
{
    return m_bAccessingValueProperty;
}

//------------------------------------------------------------------
inline	void		DbCellControl::lockValueProperty()
{
    OSL_ENSURE( !isValuePropertyLocked(), "DbCellControl::lockValueProperty: not to be nested!" );
    m_bAccessingValueProperty = sal_True;
}

//------------------------------------------------------------------
inline	void		DbCellControl::unlockValueProperty()
{
    OSL_ENSURE( isValuePropertyLocked(), "DbCellControl::lockValueProperty: not locked so far!" );
    m_bAccessingValueProperty = sal_False;
}

//==================================================================
/** a field which is bound to a column which supports the MaxTextLen property
*/
class DbLimitedLengthField : public DbCellControl
{
protected:
    DbLimitedLengthField( DbGridColumn& _rColumn );

protected:
    // DbCellControl
    virtual void implAdjustGenericFieldSetting( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel );
};

//==================================================================
class DbTextField : public DbLimitedLengthField
{
    sal_Int16		  m_nKeyType;

public:
    DbTextField(DbGridColumn& _rColumn);
    virtual void Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor );
    virtual XubString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);
    virtual CellControllerRef CreateController() const;

protected:
    // DbCellControl
    virtual sal_Bool	commitControl( );
    virtual void		updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel );
};

//==================================================================
class DbFormattedField : public DbLimitedLengthField
{	
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >	m_xSupplier;
    sal_Int16						m_nKeyType;


public:
    DbFormattedField(DbGridColumn& _rColumn);
    virtual ~DbFormattedField();


    virtual void Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor );
    virtual XubString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);
    virtual CellControllerRef CreateController() const;

protected:
    // DbCellControl
    virtual sal_Bool	commitControl( );
    virtual void		updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel );

    // OPropertyChangeListener
    virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException);
};

//==================================================================
class DbCheckBox : public DbCellControl
{
public:
    DbCheckBox(DbGridColumn& _rColumn);
    virtual void Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor );
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);
    virtual CellControllerRef CreateController() const;
    virtual void Paint(OutputDevice& rDev, const Rectangle& rRect,
                          const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant,
                          const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);

protected:
    // DbCellControl
    virtual sal_Bool	commitControl( );
    virtual void		updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel );
};

//==================================================================
class DbComboBox : public DbCellControl
{
    sal_Int16		  m_nKeyType;

public:
    DbComboBox(DbGridColumn& _rColumn);

    virtual void Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor );
    virtual XubString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);
    virtual CellControllerRef CreateController() const;

    void SetList(const ::com::sun::star::uno::Any& rItems);

protected:
    // DbCellControl
    virtual sal_Bool	commitControl( );
    virtual void		updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel );

    virtual void		implAdjustGenericFieldSetting( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel );

    // OPropertyChangeListener
    virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException);
};

//==================================================================
class DbListBox 	:public DbCellControl
{
    sal_Bool			  m_bBound	: 1;
    ::com::sun::star::uno::Sequence< ::rtl::OUString > m_aValueList;	

public:
    DbListBox(DbGridColumn& _rColumn);

    virtual void Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor );
    virtual XubString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);
    virtual CellControllerRef CreateController() const;

    void SetList(const ::com::sun::star::uno::Any& rItems);

protected:
    // DbCellControl
    virtual sal_Bool	commitControl( );
    virtual void		updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel );

    virtual void		implAdjustGenericFieldSetting( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel );

    // OPropertyChangeListener
    virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException);
};

//==================================================================
class DbPatternField : public DbCellControl
{
public:
    DbPatternField(DbGridColumn& _rColumn);
    virtual void Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor );
    virtual XubString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);
    virtual CellControllerRef CreateController() const;

protected:
    /// DbCellControl
    virtual sal_Bool	commitControl( );
    virtual void		updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel );

    virtual void		implAdjustGenericFieldSetting( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel );
};

//==================================================================
class DbSpinField : public DbCellControl
{
private:
    sal_Int16	m_nStandardAlign;

protected:
    DbSpinField( DbGridColumn& _rColumn, sal_Int16 _nStandardAlign = ::com::sun::star::awt::TextAlign::RIGHT );

public:
    virtual void						Init( Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& _rxCursor );
    virtual CellControllerRef	CreateController() const;

protected:
    virtual SpinField*	createField(
                            Window* _pParent,
                            WinBits _nFieldStyle,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel
                        ) = 0;
};

//==================================================================
class DbDateField : public DbSpinField
{
public:
    DbDateField(DbGridColumn& _rColumn);
    virtual XubString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);

protected:
    // DbCellControl
    virtual sal_Bool	commitControl( );
    virtual void		updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel );

    // DbSpinField
    virtual SpinField*	createField(
                            Window* _pParent,
                            WinBits _nFieldStyle,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel
                        );

    /// initializes everything which relates to the properties describing the numeric behaviour
    virtual void	implAdjustGenericFieldSetting( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel );
};

//==================================================================
class DbTimeField : public DbSpinField
{
public:
    DbTimeField(DbGridColumn& _rColumn);
    virtual XubString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);

protected:
    // DbCellControl
    virtual sal_Bool	commitControl( );
    virtual void		updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel );

    // DbSpinField
    virtual SpinField*	createField(
                            Window* _pParent,
                            WinBits _nFieldStyle,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel
                        );

    /// initializes everything which relates to the properties describing the numeric behaviour
    virtual void	implAdjustGenericFieldSetting( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel );
};

//==================================================================
class DbCurrencyField : public DbSpinField
{
    sal_Int16  m_nScale;

public:
    DbCurrencyField(DbGridColumn& _rColumn);
    virtual XubString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);

protected:
    double GetCurrency(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) const;

protected:
    // DbCellControl
    virtual sal_Bool	commitControl( );
    virtual void		updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel );

    // DbSpinField
    virtual SpinField*	createField(
                            Window* _pParent,
                            WinBits _nFieldStyle,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel
                        );

    /// initializes everything which relates to the properties describing the numeric behaviour
    virtual void	implAdjustGenericFieldSetting( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel );
};

//==================================================================
class DbNumericField : public DbSpinField
{
public:
    DbNumericField(DbGridColumn& _rColumn);
    virtual XubString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);

protected:
    // DbCellControl
    virtual sal_Bool	commitControl( );
    virtual void		updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel );

    // DbSpinField
    virtual SpinField*	createField(
                            Window* _pParent,
                            WinBits _nFieldStyle,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel
                        );

    /// initializes everything which relates to the properties describing the numeric behaviour
    void	implAdjustGenericFieldSetting( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel );
};

//==================================================================
class DbFilterField
        :public DbCellControl
        ,public ::binfilter::svxform::OSQLParserClient//STRIP008 		,public ::svxform::OSQLParserClient
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString >	m_aValueList;
    XubString	m_aText;
    Link	m_aCommitLink;
    sal_Int16	m_nControlClass;
    sal_Bool	m_bFilterList : 1;
    sal_Bool	m_bFilterListFilled : 1;
    sal_Bool	m_bBound : 1;

public:
    DbFilterField(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,DbGridColumn& _rColumn);
    virtual ~DbFilterField();

    virtual void Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor);
    virtual CellControllerRef CreateController() const;
    virtual void Paint(OutputDevice& rDev, const Rectangle& rRect);
    virtual void Update();

    const XubString& GetText() const {return m_aText;}
    void SetText(const XubString& rText);

    void SetCommitHdl( const Link& rLink ) { m_aCommitLink = rLink; }
    const Link& GetCommitHdl() const { return m_aCommitLink; }

protected:

    // DbCellControl
    virtual sal_Bool	commitControl( );
    virtual void		updateFromModel( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxModel );

protected:
    void SetList(const ::com::sun::star::uno::Any& rItems, sal_Bool bComboBox);
    void CreateControl(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xModel);
    DECL_LINK( OnClick, void* );
};

//==================================================================
// Base class providing the access to a grid cell
//==================================================================
class FmXGridCell : public ::cppu::OComponentHelper,
                    public ::com::sun::star::awt::XControl,
                    public ::com::sun::star::form::XBoundControl
{
protected:
    ::osl::Mutex	m_aMutex;
    DbGridColumn*			m_pColumn;
    DbCellControl*			m_pCellControl;

    virtual ~FmXGridCell();
public:
    TYPEINFO();
    FmXGridCell(DbGridColumn* pColumn, DbCellControl* pControl);


    DECLARE_UNO3_AGG_DEFAULTS(FmXGridCell, OComponentHelper);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException);

    void SetTextLineColor();
    void SetTextLineColor(const Color& _rColor);

// XTypeProvider
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

// OComponentHelper
    virtual void SAL_CALL disposing();

// ::com::sun::star::lang::XComponent
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException){OComponentHelper::dispose();}
    virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener)throw(::com::sun::star::uno::RuntimeException)		{ OComponentHelper::addEventListener(aListener);}
    virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener)throw(::com::sun::star::uno::RuntimeException)		{ OComponentHelper::removeEventListener(aListener);}

// ::com::sun::star::awt::XControl
    virtual void SAL_CALL setContext(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Context) throw(::com::sun::star::uno::RuntimeException){}
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL getContext() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL createPeer(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent) throw(::com::sun::star::uno::RuntimeException){}

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > SAL_CALL getPeer() throw (::com::sun::star::uno::RuntimeException) {return ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > ();}
    virtual sal_Bool SAL_CALL setModel(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& Model) throw (::com::sun::star::uno::RuntimeException) {return sal_False;}
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > SAL_CALL getModel() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XView > SAL_CALL getView() throw (::com::sun::star::uno::RuntimeException) {return ::com::sun::star::uno::Reference< ::com::sun::star::awt::XView > ();}
    virtual void SAL_CALL setDesignMode(sal_Bool bOn) throw (::com::sun::star::uno::RuntimeException) {}
    virtual sal_Bool SAL_CALL isDesignMode() throw (::com::sun::star::uno::RuntimeException) {return sal_False;}
    virtual sal_Bool SAL_CALL isTransparent() throw (::com::sun::star::uno::RuntimeException) {return sal_False;}

// ::com::sun::star::form::XBoundControl
    virtual sal_Bool SAL_CALL getLock() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setLock(sal_Bool _bLock) throw(::com::sun::star::uno::RuntimeException);

    sal_Bool Commit() {return m_pCellControl->Commit();}
    void ImplInitSettings(Window* pParent, sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground)
        { m_pCellControl->ImplInitSettings(pParent, bFont, bForeground, bBackground); }

    sal_Bool isAlignedController() const { return m_pCellControl->isAlignedController(); }
    void AlignControl(sal_Int16 nAlignment)
        { m_pCellControl->AlignControl(nAlignment);}
};

//==================================================================
class FmXDataCell : public FmXGridCell
{
public:
    TYPEINFO();
    FmXDataCell(DbGridColumn* pColumn, DbCellControl* pControl):FmXGridCell(pColumn, pControl){}

    virtual void Paint(OutputDevice& rDev,
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
public:
    TYPEINFO();
    FmXTextCell(DbGridColumn* pColumn, DbCellControl* pControl):FmXDataCell(pColumn, pControl){}

    virtual void Paint(OutputDevice& rDev,
               const Rectangle& rRect,
               const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& xField,
               const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);

    XubString GetText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant,
                   const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter,
                   Color** ppColor = NULL)
            {return m_pCellControl->GetFormatText(_xVariant, xFormatter, ppColor);}
};

//==================================================================
class FmXEditCell : public FmXTextCell,
                    public ::com::sun::star::awt::XTextComponent
{
protected:
    ::cppu::OInterfaceContainerHelper	m_aTextListeners;
    Edit*								m_pEdit;

    virtual ~FmXEditCell();
public:
    FmXEditCell(DbGridColumn* pColumn, DbCellControl* pControl);
    

    DECLARE_UNO3_AGG_DEFAULTS(FmXEditCell, FmXTextCell);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException);

// XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

// OComponentHelper
    virtual void SAL_CALL disposing();

// ::com::sun::star::awt::XTextComponent
    virtual void SAL_CALL addTextListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener >& l) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTextListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener >& l) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setText(const ::rtl::OUString& aText) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertText(const ::com::sun::star::awt::Selection& Sel, const ::rtl::OUString& Text) throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getText() throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getSelectedText() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setSelection(const ::com::sun::star::awt::Selection& aSelection) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Selection SAL_CALL getSelection() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isEditable() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setEditable(sal_Bool bEditable) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setMaxTextLen(sal_Int16 nLen) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getMaxTextLen() throw(::com::sun::star::uno::RuntimeException);

protected:
    DECL_LINK( OnTextChanged, void* );
};

//==================================================================
class FmXCheckBoxCell : public FmXDataCell,
                        public ::com::sun::star::awt::XCheckBox
{
    ::cppu::OInterfaceContainerHelper	m_aItemListeners;
    CheckBox*							m_pBox;
protected:
    virtual ~FmXCheckBoxCell();
public:
    FmXCheckBoxCell(DbGridColumn* pColumn, DbCellControl* pControl);
    

// UNO
    DECLARE_UNO3_AGG_DEFAULTS(FmXCheckBoxCell, FmXDataCell);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

// OComponentHelper
    virtual void SAL_CALL disposing();

// ::com::sun::star::awt::XCheckBox
    virtual void SAL_CALL addItemListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeItemListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getState() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setState(sal_Int16 n) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setLabel(const ::rtl::OUString& Label) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL enableTriState(sal_Bool b) throw(::com::sun::star::uno::RuntimeException);

protected:
    DECL_LINK( OnClick, void* );
};

//==================================================================
class FmXListBoxCell : public FmXTextCell,
                       public ::com::sun::star::awt::XListBox
{
    ::cppu::OInterfaceContainerHelper	m_aItemListeners,
                                        m_aActionListeners;
    ListBox*							m_pBox;
protected:
    virtual ~FmXListBoxCell();
public:
    FmXListBoxCell(DbGridColumn* pColumn, DbCellControl* pControl);
    

    DECLARE_UNO3_AGG_DEFAULTS(FmXListBoxCell, FmXTextCell);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

// OComponentHelper
    virtual void SAL_CALL disposing();

// ::com::sun::star::awt::XListBox
    virtual void SAL_CALL addItemListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeItemListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addActionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeActionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& l) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addItem(const ::rtl::OUString& aItem, sal_Int16 nPos) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addItems(const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aItems, sal_Int16 nPos) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeItems(sal_Int16 nPos, sal_Int16 nCount) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getItemCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getItem(sal_Int16 nPos) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getItems() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getSelectedItemPos() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int16 > SAL_CALL getSelectedItemsPos() throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getSelectedItem() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSelectedItems() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL SAL_CALL selectItemPos(sal_Int16 nPos, sal_Bool bSelect) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL SAL_CALL selectItemsPos(const ::com::sun::star::uno::Sequence< sal_Int16 >& aPositions, sal_Bool bSelect) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL SAL_CALL selectItem(const ::rtl::OUString& aItem, sal_Bool bSelect) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isMutipleMode() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL SAL_CALL setMultipleMode(sal_Bool bMulti) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getDropDownLineCount() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL SAL_CALL setDropDownLineCount(sal_Int16 nLines) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL SAL_CALL makeVisible(sal_Int16 nEntry) throw(::com::sun::star::uno::RuntimeException);

protected:
    DECL_LINK( OnSelect, void* );
    DECL_LINK( OnDoubleClick, void* );
};

//==================================================================
class FmXFilterCell :public FmXGridCell
                    ,public ::com::sun::star::awt::XTextComponent
                    ,public ::com::sun::star::lang::XUnoTunnel
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

// XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

// helpers for XUnoTunnel
    static const ::com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static FmXFilterCell* getImplementation(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxObject);

//	Painting the filter text
    void Paint(OutputDevice& rDev, const Rectangle& rRect);
    void Update(){m_pCellControl->Update();}

// OComponentHelper
    virtual void SAL_CALL disposing();

// ::com::sun::star::awt::XTextComponent
    virtual void SAL_CALL addTextListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener >& l) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTextListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener >& l) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setText(const ::rtl::OUString& aText) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertText(const ::com::sun::star::awt::Selection& Sel, const ::rtl::OUString& Text) throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getText() throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getSelectedText() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setSelection(const ::com::sun::star::awt::Selection& aSelection) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Selection SAL_CALL getSelection() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isEditable() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setEditable(sal_Bool bEditable) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setMaxTextLen(sal_Int16 nLen) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getMaxTextLen() throw(::com::sun::star::uno::RuntimeException);

protected:
    DECL_LINK( OnCommit, void* );
};

}//end of namespace binfilter
#endif // _SVX_GRIDCELL_HXX

