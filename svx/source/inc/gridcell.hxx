/*************************************************************************
 *
 *  $RCSfile: gridcell.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-20 14:12:06 $
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

#ifndef _SVX_GRIDCELL_HXX
#define _SVX_GRIDCELL_HXX

#ifndef _SVX_GRIDCTRL_HXX
#include "gridctrl.hxx"
#endif

#ifndef _SVX_FMTOOLS_HXX
#include "fmtools.hxx"
#endif

#ifndef _CPPUHELPER_COMPONENT_HXX_
#include <cppuhelper/component.hxx>
#endif

#ifndef _COM_SUN_STAR_SDB_XCOLUMN_HPP_
#include <com/sun/star/sdb/XColumn.hpp>
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
#ifndef _COM_SUN_STAR_AWT_XCONTROLMODEL_HPP_
#include <com/sun/star/awt/XControlModel.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCHECKBOX_HPP_
#include <com/sun/star/awt/XCheckBox.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XFASTPROPERTYSET_HPP_
#include <com/sun/star/beans/XFastPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#ifndef _COMPHELPER_PROPERTY_MULTIPLEX_HXX_
#include <comphelper/propmultiplex.hxx>
#endif

class DbCellControl;
class Edit;
class FmXGridCell;

//==================================================================
// DbGridColumn, Spaltenbeschreibung
//==================================================================
class DbGridColumn
{
    friend class DbGridControl;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xField;       // Verbindung zum Datenbankfeld
    DbCellControllerRef m_xController;  // Struktur zum Verwalten der Controls fuer eine Spalte
                                        // diese wird von der DbBrowseBox auf die jeweiligen Zellen
                                        // einer Spalte positioniert
    FmXGridCell*        m_pCell;

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
    sal_Bool                m_bRequired : 1;
    sal_Bool                m_bReadOnly : 1;
    sal_Bool                m_bAutoValue : 1;
    sal_Bool                m_bInSave : 1;
    sal_Bool                m_bNumeric : 1;
    sal_Bool                m_bObject : 1;  // Verweist die Column auf ein Object Datentyp?
    sal_Bool                m_bHidden : 1;
    sal_Bool                m_bLocked : 1;
    sal_Bool                m_bDateTime : 1;

    static DbCellControllerRef s_xEmptyController;
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
        ,m_bLocked(sal_False) {}

    ~DbGridColumn();

    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& getModel() const {return m_xModel;}
    void  setModel(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  _xModel) {m_xModel = _xModel;}


    sal_uInt16  GetId() const {return m_nId;}
    sal_Bool    IsRequired() const {return m_bRequired;}
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
    const   DbCellControllerRef& GetController() const {return m_bLocked ? s_xEmptyController : m_xController;}
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

    XubString  GetCellText(const DbGridRow* pRow, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) const;
    XubString  GetCellText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& xField, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) const;

    void    SetReadOnly(sal_Bool bRead){m_bReadOnly = bRead;}
    void    SetObject(sal_Int16 nPos) {m_bObject = m_bReadOnly = sal_True; m_nFieldPos = nPos;}
    void    ImplInitSettings(Window* pParent, sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground);

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
};

//==================================================================
// DbCellControl, liefert die Daten fuer einen CellController
// wird in der Regel nur für komplexe Controls wie z.B ComboBoxen
// benoetigt
//==================================================================
class DbCellControl
{
protected:
    DbGridColumn& m_rColumn;
    Window*       m_pWindow;
    Window*       m_pPainter;
    sal_Bool          m_bTransparent : 1;
    sal_Bool          m_bAlignedController : 1;

public:
    DbCellControl(DbGridColumn& _rColumn, sal_Bool _bText = sal_True)
        :m_rColumn(_rColumn)
        ,m_pWindow(NULL)
        ,m_pPainter(NULL)
        ,m_bTransparent(sal_False)
        ,m_bAlignedController(sal_True){}

    virtual ~DbCellControl();
    Window* GetControl() const {return m_pWindow;}

    sal_Bool IsAlignedController() const { return m_bAlignedController; }
    void AlignControl(sal_Int16 nAlignment);

    // Initialisieren bevor ein Control angezeigt wird
    virtual void Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor);
    virtual DbCellControllerRef CreateController() const = 0;

    // Schreiben des Wertes in das Model
    virtual sal_Bool Commit() = 0;

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
};

//==================================================================
class DbFormattedField : public DbCellControl, public ::comphelper::OPropertyChangeListener
{
protected:
    ::comphelper::OPropertyChangeMultiplexer*       m_pFormatListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >  m_xSupplier;
    sal_Int16                       m_nKeyType;

    ::osl::Mutex    m_aDummy;
        // only for the base class OPropertyChangeListener, in UNO3 we can't use the SolarMutex anymore

public:
    DbFormattedField(DbGridColumn& _rColumn);
    virtual ~DbFormattedField();

    virtual void Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor );
    virtual XubString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);
    virtual DbCellControllerRef CreateController() const;
    virtual sal_Bool Commit();

protected:
    virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException);
};

//==================================================================
class DbTextField : public DbCellControl
{
    sal_Int16         m_nKeyType;
public:
    DbTextField(DbGridColumn& _rColumn);
    virtual void Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor );
    virtual XubString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);
    virtual DbCellControllerRef CreateController() const;
    virtual sal_Bool Commit();
};

//==================================================================
class DbCheckBox : public DbCellControl
{
public:
    DbCheckBox(DbGridColumn& _rColumn):DbCellControl(_rColumn, sal_True) { m_bAlignedController = sal_False; };
    virtual void Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor );
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);
    virtual DbCellControllerRef CreateController() const;
    virtual sal_Bool Commit();
    virtual void Paint(OutputDevice& rDev, const Rectangle& rRect,
                          const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant,
                          const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);

};

//==================================================================
class DbComboBox    :public DbCellControl
                    ,public ::comphelper::OPropertyChangeListener
{
    sal_Int16         m_nKeyType;
    ::osl::Mutex    m_aDummy;
        // only for the base class OPropertyChangeListener, in UNO3 we can't use the SolarMutex anymore

public:
    DbComboBox(DbGridColumn& _rColumn);

    virtual void Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor );
    virtual XubString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);
    virtual DbCellControllerRef CreateController() const;
    virtual sal_Bool Commit();

// OPropertyChangeListener
    virtual void _propertyChanged( const ::com::sun::star::beans::PropertyChangeEvent& ) throw(::com::sun::star::uno::RuntimeException);

    void SetList(const ::com::sun::star::uno::Any& rItems);
};

//==================================================================
class DbListBox : public DbCellControl,
                  public ::comphelper::OPropertyChangeListener
{
    sal_Bool              m_bBound  : 1;
    ::com::sun::star::uno::Sequence< ::rtl::OUString > m_aValueList;
    ::osl::Mutex    m_aDummy;
        // only for the base class OPropertyChangeListener, in UNO3 we can't use the SolarMutex anymore

public:
    DbListBox(DbGridColumn& _rColumn);

    virtual void Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor );
    virtual XubString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);
    virtual DbCellControllerRef CreateController() const;
    virtual sal_Bool Commit();

// OPropertyChangeListener
    virtual void _propertyChanged( const ::com::sun::star::beans::PropertyChangeEvent& ) throw(::com::sun::star::uno::RuntimeException);

    void SetList(const ::com::sun::star::uno::Any& rItems);
};

//==================================================================
class DbPatternField : public DbCellControl
{
public:
    DbPatternField(DbGridColumn& _rColumn):DbCellControl(_rColumn){};
    virtual void Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor );
    virtual XubString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);
    virtual DbCellControllerRef CreateController() const;
    virtual sal_Bool Commit();
};

//==================================================================
class DbDateField : public DbCellControl
{
public:
    DbDateField(DbGridColumn& _rColumn):DbCellControl(_rColumn){};
    virtual void Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor );
    virtual XubString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);
    virtual DbCellControllerRef CreateController() const;
    virtual sal_Bool Commit();
};

//==================================================================
class DbTimeField : public DbCellControl
{
public:
    DbTimeField(DbGridColumn& _rColumn):DbCellControl(_rColumn){};
    virtual void Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor );
    virtual XubString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);
    virtual DbCellControllerRef CreateController() const;
    virtual sal_Bool Commit();
};

//==================================================================
class DbCurrencyField : public DbCellControl
{
    sal_Int16  m_nScale;

public:
    DbCurrencyField(DbGridColumn& _rColumn):DbCellControl(_rColumn),m_nScale(0){};
    virtual void Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor );
    virtual XubString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);
    virtual DbCellControllerRef CreateController() const;
    virtual sal_Bool Commit();

protected:
    double GetCurrency(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter) const;
};

//==================================================================
class DbNumericField : public DbCellControl
{
public:
    DbNumericField(DbGridColumn& _rColumn):DbCellControl(_rColumn){};
    virtual void Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor );
    virtual XubString GetFormatText(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter, Color** ppColor = NULL);
    virtual void UpdateFromField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _xVariant, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xFormatter);
    virtual DbCellControllerRef CreateController() const;
    virtual sal_Bool Commit();
};

//==================================================================
class DbFilterField : public DbCellControl
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > m_aValueList;
    XubString   m_aText;
    Link    m_aCommitLink;
    sal_Int16   m_nControlClass;
    sal_Bool    m_bFilterList : 1;
    sal_Bool    m_bFilterListFilled : 1;
    sal_Bool    m_bBound : 1;

public:
    DbFilterField(DbGridColumn& _rColumn);
    virtual ~DbFilterField();

    virtual void Init(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xCursor);
    virtual DbCellControllerRef CreateController() const;
    virtual void Paint(OutputDevice& rDev, const Rectangle& rRect);
    virtual sal_Bool Commit();
    virtual void Update();

    const XubString& GetText() const {return m_aText;}
    void SetText(const XubString& rText);

    void SetCommitHdl( const Link& rLink ) { m_aCommitLink = rLink; }
    const Link& GetCommitHdl() const { return m_aCommitLink; }

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
    ::osl::Mutex    m_aMutex;
    DbGridColumn*           m_pColumn;
    DbCellControl*          m_pCellControl;

public:
    TYPEINFO();
    FmXGridCell(DbGridColumn* pColumn, DbCellControl* pControl);
    virtual ~FmXGridCell();

    DECLARE_UNO3_AGG_DEFAULTS(FmXGridCell, OComponentHelper);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException);

// XTypeProvider
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

// OComponentHelper
    virtual void SAL_CALL disposing();

// ::com::sun::star::lang::XComponent
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException){OComponentHelper::dispose();}
    virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener)throw(::com::sun::star::uno::RuntimeException)       { OComponentHelper::addEventListener(aListener);}
    virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener)throw(::com::sun::star::uno::RuntimeException)        { OComponentHelper::removeEventListener(aListener);}

// ::com::sun::star::awt::XControl
    virtual void SAL_CALL setContext(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Context) throw(::com::sun::star::uno::RuntimeException){}
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL getContext() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL createPeer(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent) throw(::com::sun::star::uno::RuntimeException){}

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > SAL_CALL getPeer() {return ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > ();}
    virtual sal_Bool SAL_CALL setModel(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& Model) {return sal_False;}
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > SAL_CALL getModel();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XView > SAL_CALL getView() {return ::com::sun::star::uno::Reference< ::com::sun::star::awt::XView > ();}
    virtual void SAL_CALL setDesignMode(sal_Bool bOn) {}
    virtual sal_Bool SAL_CALL isDesignMode() {return sal_False;}
    virtual sal_Bool SAL_CALL isTransparent() {return sal_False;}

// ::com::sun::star::form::XBoundControl
    virtual sal_Bool SAL_CALL getLock() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setLock(sal_Bool _bLock) throw(::com::sun::star::uno::RuntimeException);

    sal_Bool Commit() {return m_pCellControl->Commit();}
    void ImplInitSettings(Window* pParent, sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground)
        { m_pCellControl->ImplInitSettings(pParent, bFont, bForeground, bBackground); }

    sal_Bool IsAlignedController() const { return m_pCellControl->IsAlignedController(); }
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
    ::cppu::OInterfaceContainerHelper   m_aTextListeners;
    Edit*                               m_pEdit;

public:
    FmXEditCell(DbGridColumn* pColumn, DbCellControl* pControl);
    ~FmXEditCell();

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
    ::cppu::OInterfaceContainerHelper   m_aItemListeners;
    CheckBox*                           m_pBox;

public:
    FmXCheckBoxCell(DbGridColumn* pColumn, DbCellControl* pControl);
    ~FmXCheckBoxCell();

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
    ::cppu::OInterfaceContainerHelper   m_aItemListeners,
                                        m_aActionListeners;
    ListBox*                            m_pBox;

public:
    FmXListBoxCell(DbGridColumn* pColumn, DbCellControl* pControl);
    ~FmXListBoxCell();

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
public:
    TYPEINFO();
    FmXFilterCell(DbGridColumn* pColumn = NULL, DbCellControl* pControl = NULL);
    ~FmXFilterCell();

    DECLARE_UNO3_AGG_DEFAULTS(FmXFilterCell, FmXGridCell);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

// XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

// helpers for XUnoTunnel
    static const ::com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static FmXFilterCell* getImplementation(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxObject);

//  Painting the filter text
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

#endif // _SVX_GRIDCELL_HXX

