/*************************************************************************
 *
 *  $RCSfile: fmtools.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: fs $ $Date: 2000-11-09 10:18:46 $
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
#ifndef _SVX_FMTOOLS_HXX
#define _SVX_FMTOOLS_HXX

#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSERFACTORY_HPP_
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_PRIVILEGE_HPP_
#include <com/sun/star/sdbcx/Privilege.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDATADESCRIPTORFACTORY_HPP_
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOLUMN_HPP_
#include <com/sun/star/sdb/XColumn.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOLUMNUPDATE_HPP_
#include <com/sun/star/sdb/XColumnUpdate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_SQLERROREVENT_HPP_
#include <com/sun/star/sdb/SQLErrorEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETUPDATE_HPP_
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XROWLOCATE_HPP_
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTION_HPP_
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTOR_HPP_
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XINTERCEPTORINFO_HPP_
#include <com/sun/star/frame/XInterceptorInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FEATURESTATEEVENT_HPP_
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_SCRIPTEVENTDESCRIPTOR_HPP_
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLMODEL_HPP_
#include <com/sun/star/awt/XControlModel.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTSLANT_HPP_
#include <com/sun/star/awt/FontSlant.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTUNDERLINE_HPP_
#include <com/sun/star/awt/FontUnderline.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTSTRIKEOUT_HPP_
#include <com/sun/star/awt/FontStrikeout.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _SV_WINTYPES_HXX //autogen
#include <vcl/wintypes.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif

#ifndef _SVSTDARR_ULONGS
#define _SVSTDARR_ULONGS
#include <svtools/svstdarr.hxx>
#endif

#ifndef _SFXCTRLITEM_HXX //autogen wg. SfxControllerItem
#include <sfx2/ctrlitem.hxx>
#endif

//  #ifndef _USR_ITERHLP_HXX //autogen wg. OInterfaceContainerHelper
//  #include <usr/iterhlp.hxx>
//  #endif

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

#ifndef _DATE_HXX
#include <tools/date.hxx>
#endif
#ifndef _TOOLS_TIME_HXX
#include <tools/time.hxx>
#endif
#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif

#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTINPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XPERSISTOBJECT_HPP_
#include <com/sun/star/io/XPersistObject.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATS_HPP_
#include <com/sun/star/util/XNumberFormats.hpp>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif
#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE3_HXX_
#include <cppuhelper/compbase3.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef _CPPUHELPER_COMPONENT_HXX_
#include <cppuhelper/component.hxx>
#endif

#ifndef _FM_IMPLEMENTATION_IDS_HXX_
#include "fmimplids.hxx"
#endif

#ifndef _FM_STATIC_HXX_
#include "fmstatic.hxx"
#endif


//==================================================================
// allgemeine Typen
//==================================================================
// displaying a database exception for the user
// display info about a simple ::com::sun::star::sdbc::SQLException
void displayException(const ::com::sun::star::sdbc::SQLException&, WinBits nStyle = WB_OK | WB_DEF_OK);
// display info about chained SQLExceptions
void displayException(const ::com::sun::star::sdb::SQLErrorEvent&, WinBits nStyle = WB_OK | WB_DEF_OK);


// StringConversion
#define U(x)                    \
    ::rtl::OStringToOUString( x, RTL_TEXTENCODING_ASCII_US )
#define S(aUniCode)             \
    ::rtl::OUStringToOString(aUniCode, RTL_TEXTENCODING_ASCII_US)

// modes for the XModeSelector
DECLARE_CONSTASCII_USTRING(DATA_MODE);
DECLARE_CONSTASCII_USTRING(FILTER_MODE);


// Kopieren von Persistenten Objecten
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> clone(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XPersistObject>& _xObj);
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> cloneUsingProperties(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XPersistObject>& _xObj);
void CloneForms(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer>& _xSource, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer>& _xDest);

// Suchen eines Wertes in einer StringListe
// Fehler 0xFFFFFFFF (LIST_ENTRY_NOTFOUND)
//------------------------------------------------------------------------------
sal_uInt32 findValue(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>& rList, const ::com::sun::star::uno::Any& rValue);

::com::sun::star::uno::Sequence<sal_Int16> findValueINT16(const ::com::sun::star::uno::Sequence< ::rtl::OUString>& rList, const ::rtl::OUString& rValue, sal_Bool bOnlyFirst=sal_False );
::com::sun::star::uno::Sequence<sal_Int16> findValue(const ::com::sun::star::uno::Sequence< ::rtl::OUString>& rList, const ::rtl::OUString& rValue, sal_Bool bOnlyFirst=sal_False );

sal_uInt32 findValue1(const ::com::sun::star::uno::Sequence< ::rtl::OUString>& rList, const ::rtl::OUString& rValue);

// geht von einer sortierten !!! sequence aus
sal_Bool hasString(const ::rtl::OUString& aStr, const ::com::sun::star::uno::Sequence< ::rtl::OUString>& rList);

sal_Int32 findPos(const ::rtl::OUString& aStr, const ::com::sun::star::uno::Sequence< ::rtl::OUString>& rList);

//  void checkArg(const ::com::sun::star::uno::Any& Element, Reflection* pRefl) throw( ::com::sun::star::lang::IllegalArgumentException );

// Suchen in einer Indexliste nach einem Element
sal_Bool  searchElement(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& xCont, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xElement);

sal_Int32 getElementPos(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& xCont, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xElement);
String getFormComponentAccessPath(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _xElement);
String getFormComponentAccessPath(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _xElement, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _rTopLevelElement);
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> getElementFromAccessPath(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& _xParent, const String& _rRelativePath);


::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel> getXModel(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xIface);

::rtl::OUString getLabelName(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& xControlModel);
::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> findConnection(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xParent);

// date conversion
extern Date STANDARD_DATE;
double ToStandardDate(const Date& rNullDate, double rVal);
double ToNullDate(const Date& rNullDate, double rVal);


// ===================================================================================================
// = class CursorWrapper - eine Hilfsklasse, die parallel mit je einem ::com::sun::star::uno::Reference<XDatabaseUpdateCursor>,
//                          XDatabaseBookmarkCursor und XDatabaseDirectCursor arbeitet
// ===================================================================================================

class CursorWrapper
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>                m_xGeneric;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>               m_xMoveOperations;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XRowLocate>              m_xBookmarkOperations;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier>        m_xColumnsSupplier;

public:
    // Construction/Destruction
    CursorWrapper() { }
    CursorWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _rxCursor, sal_Bool bUseCloned = sal_False);
    CursorWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _rxCursor, sal_Bool bUseCloned = sal_False);
        // bei bUseCloned == sal_True wird der Cursor ueber das XCloneable-Interface (dass er besitzen muss) gedoubled und
        // erst dann benutzt

    friend bool operator==(const CursorWrapper& lhs, const CursorWrapper& rhs)
    {
        return lhs.m_xGeneric.get() == rhs.m_xGeneric.get();
    }

    sal_Bool is() const { return m_xMoveOperations.is(); }
    sal_Bool Is() const { return m_xMoveOperations.is(); }

    CursorWrapper* operator ->() { return this; }
    operator const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& () const{ return m_xGeneric; }

    // 'Konvertierungen'
    const CursorWrapper& operator=(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& xCursor);
    operator const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& () const          { return m_xMoveOperations; }
    operator const ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XRowLocate>& () const         { return m_xBookmarkOperations; }
    operator const ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier>& () const   { return m_xColumnsSupplier; }
    operator const ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier> () const{ return m_xColumnsSupplier; }


    // das normale queryInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& type) throw ( ::com::sun::star::uno::RuntimeException )
    { return m_xMoveOperations->queryInterface(type); }

    // ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XRowLocate>
    ::com::sun::star::uno::Any getBookmark(void)
        throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
    { return m_xBookmarkOperations->getBookmark(); }
    sal_Bool moveToBookmark(const ::com::sun::star::uno::Any& bookmark) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException ) { return m_xBookmarkOperations->moveToBookmark(bookmark); }
    sal_Bool moveRelativeToBookmark(const ::com::sun::star::uno::Any& bookmark, sal_Int32 rows) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException ) { return m_xBookmarkOperations->moveRelativeToBookmark(bookmark, rows); }
    sal_Int32 compareBookmarks(const ::com::sun::star::uno::Any& first, const ::com::sun::star::uno::Any& second) const throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException ) { return m_xBookmarkOperations->compareBookmarks(first, second); }
    sal_Int32 hasOrderedBookmarks(void) const throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException ) { return m_xBookmarkOperations->hasOrderedBookmarks(); }
    sal_Int32 hashBookmark(const ::com::sun::star::uno::Any& bookmark) const throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException ) { return m_xBookmarkOperations->hashBookmark(bookmark); }

    // ::com::sun::star::sdbc::XResultSet
    sal_Bool isBeforeFirst() const              { return m_xMoveOperations->isBeforeFirst(); }
    sal_Bool isAfterLast() const                { return m_xMoveOperations->isAfterLast(); }
    sal_Bool isFirst() const                    { return m_xMoveOperations->isFirst(); }
    sal_Bool isLast() const                     { return m_xMoveOperations->isLast(); }
//  void moveBeforeFirst()                  { m_xMoveOperations->moveBeforeFirst(); }
//  void moveAfterLast()                    { m_xMoveOperations->moveAfterLast(); }
    sal_Bool first()                            { return m_xMoveOperations->first(); }
    sal_Bool last()                             { return m_xMoveOperations->last(); }
    sal_Int32 getRow() const                    { return m_xMoveOperations->getRow(); }
    sal_Bool absolute(sal_Int32 nPosition)          { return m_xMoveOperations->absolute(nPosition); }
    sal_Bool relative(sal_Int32 nCount)             { return m_xMoveOperations->relative(nCount); }
    sal_Bool previous()                         { return m_xMoveOperations->previous(); }
    sal_Bool next()                             { return m_xMoveOperations->next(); }
    void refreshRow()                       { m_xMoveOperations->refreshRow(); }
    sal_Bool rowUpdated()                       { return m_xMoveOperations->rowUpdated(); }
    sal_Bool rowInserted()                      { return m_xMoveOperations->rowInserted(); }
    sal_Bool rowDeleted()                       { return m_xMoveOperations->rowDeleted(); }
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> getStatement()         { return m_xMoveOperations->getStatement(); }
    // ::com::sun::star::sdbcx::XColumnsSupplier
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> getColumns() const throw( ::com::sun::star::uno::RuntimeException ) { return m_xColumnsSupplier->getColumns(); }
private:
    void ImplConstruct(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _rxCursor, sal_Bool bUseCloned);
};


//==================================================================
// FmXSequenceIndexAccess - a class which wraps a ::com::sun::star::uno::Sequence so it's
// elements may be accessed via XIndexAccess-methods
//==================================================================

template <class ElementType>
class FmXSequenceIndexAccess : public ::cppu::WeakImplHelper1< ::com::sun::star::container::XIndexAccess>
{
    ::com::sun::star::uno::Sequence<ElementType>    m_aElements;

public:
    FmXSequenceIndexAccess(const ::com::sun::star::uno::Sequence<ElementType>& _rElements) : m_aElements(_rElements) { }

    // UNO
    DECLARE_UNO3_DEFAULTS(FmXSequenceIndexAccess, cppu::WeakImplHelper1< ::com::sun::star::container::XIndexAccess> );
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& type) throw ( ::com::sun::star::uno::RuntimeException );

    // XIndexAccess
    virtual sal_Int32 getCount(void) const throw( ::com::sun::star::uno::RuntimeException ) { return m_aElements.getLength(); }
    virtual ::com::sun::star::uno::Any getByIndex(sal_Int32 Index) const throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException )
    {   // we implement this in-place as MSVC41 doesn't like a non-in-place template implementation with a throw-clause
        if ((Index < 0) || (Index >= getCount()))
        {
            throw ::com::sun::star::lang::IndexOutOfBoundsException();
        }

        return ::com::sun::star::uno::Any(m_aElements.getConstArray() + Index, ::getReflection((ElementType*)NULL));
    }

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException)
    { return ::getCppuType((ElementType*)NULL); }
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException)
    { return m_aElements.getLength() > 0; }

protected:
    ~FmXSequenceIndexAccess() { }
};

//------------------------------------------------------------------
template <class ElementType>
::com::sun::star::uno::Any SAL_CALL FmXSequenceIndexAccess<ElementType>::queryInterface( const ::com::sun::star::uno::Type& type) throw ( ::com::sun::star::uno::RuntimeException )
{
    return cppu::WeakImplHelper1< ::com::sun::star::container::XIndexAccess >::queryInterface(type);
}

//==================================================================
// Einfacher Adapter fuer Components
//==================================================================
class FmXDisposeMultiplexer;
class FmXDisposeListener
{
    friend class FmXDisposeMultiplexer;

    FmXDisposeMultiplexer*  m_pAdapter;
    ::osl::Mutex&   m_rMutex;

public:
    FmXDisposeListener(::osl::Mutex& _rMutex) : m_pAdapter(NULL), m_rMutex(_rMutex) { }
    virtual ~FmXDisposeListener();

    virtual void disposing(const ::com::sun::star::lang::EventObject& _rEvent, sal_Int16 _nId) throw( ::com::sun::star::uno::RuntimeException ) = 0;

protected:
    void setAdapter(FmXDisposeMultiplexer* pAdapter);
};

typedef ::cppu::WeakImplHelper1< ::com::sun::star::lang::XEventListener> FmXDisposeMultiplexer_x;
//==============================================================================


class FmXDisposeMultiplexer :public ::cppu::WeakImplHelper1< ::com::sun::star::lang::XEventListener>
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>       m_xObject;
    FmXDisposeListener* m_pListener;
    sal_Int16               m_nId;

    virtual ~FmXDisposeMultiplexer();
public:
    FmXDisposeMultiplexer(FmXDisposeListener* _pListener, const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>& _rxObject, sal_Int16 _nId = -1);

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    void dispose();
};

//  ==================================================================

//==============================================================================
// FmSlotDispatch - some kind of translator between the Sfx-Slots and the UNO-dispatchers
//==============================================================================

typedef ::cppu::WeakImplHelper1< ::com::sun::star::frame::XDispatch >   FmSlotDispatch_x;
class FmSlotDispatch
            :public SfxControllerItem
            ,public ::cppu::WeakImplHelper2< ::com::sun::star::frame::XDispatch, ::com::sun::star::lang::XComponent>
            //  ,public XComponent
{
protected:
    ::cppu::OInterfaceContainerHelper   m_aDisposeListeners;
    ::cppu::OInterfaceContainerHelper   m_aStatusListeners;

    Link    m_aExecutor;
    ::com::sun::star::util::URL     m_aUrl;             // the URL we're responsible for
    ::osl::Mutex                    m_aAccessSafety;
    sal_Int16   m_nSlot;            // the slot 'translation' of the URL

public:
    const ::com::sun::star::util::URL&  getUrl() const { return m_aUrl; }
    sal_Int16       getSlot() const { return m_nSlot; }

    void setExecutor(const Link& aExecutor) { m_aExecutor = aExecutor; }
    Link getExecutor() const { return m_aExecutor; }

public:
    FmSlotDispatch(const ::com::sun::star::util::URL& rUrl, sal_Int16 nSlotId, SfxBindings& rBindings);
    virtual ~FmSlotDispatch();

    // ::com::sun::star::frame::XDispatch
    virtual void SAL_CALL dispatch( const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl, const ::com::sun::star::util::URL& aURL ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl, const ::com::sun::star::util::URL& aURL ) throw(::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);

protected:
    // SfxControllerItem
    virtual void StateChanged(sal_Int16 nSID, SfxItemState eState, const SfxPoolItem* pState);

    ::com::sun::star::frame::FeatureStateEvent BuildEvent(SfxItemState eState, const SfxPoolItem* pState);

    virtual void NotifyState(SfxItemState eState, const SfxPoolItem* pState, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener>& rListener = ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener>());
        // the default implementations notifies the given listeners (or all available listeners) with a statusChanged
};

//========================================================================
//= dispatch interception helper classes
//========================================================================

//------------------------------------------------------------------------
//- FmDispatchInterceptor
//------------------------------------------------------------------------
class FmDispatchInterceptor
{
public:
    FmDispatchInterceptor() { }

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch> interceptedQueryDispatch(sal_uInt16 _nId,
        const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw( ::com::sun::star::uno::RuntimeException ) = 0;

    virtual ::osl::Mutex* getInterceptorMutex() = 0;
};

//------------------------------------------------------------------------
//- FmXDispatchInterceptorImpl
//------------------------------------------------------------------------
typedef ::cppu::WeakComponentImplHelper3<   ::com::sun::star::frame::XDispatchProviderInterceptor
                                        ,   ::com::sun::star::lang::XEventListener
                                        ,   ::com::sun::star::frame::XInterceptorInfo
                                        >   FmXDispatchInterceptorImpl_BASE;

class FmXDispatchInterceptorImpl : public FmXDispatchInterceptorImpl_BASE
{
    ::osl::Mutex                    m_aFallback;
    ::form::OImplementationIdsRef   m_aHoldImplIdHelper;

    // the component which's dispatches we're intercepting
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterception>   m_xIntercepted;
    // the real interceptor
    FmDispatchInterceptor*          m_pMaster;

    // chaining
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider>           m_xSlaveDispatcher;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider>           m_xMasterDispatcher;

    // our id
    sal_Int16                       m_nId;

    ::com::sun::star::uno::Sequence< ::rtl::OUString >
                                    m_aInterceptedURLSchemes;

    virtual ~FmXDispatchInterceptorImpl();

public:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterception> getIntercepted() const { return m_xIntercepted; }

public:
    FmXDispatchInterceptorImpl(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterception>& _rToIntercept,
        FmDispatchInterceptor* _pMaster,
        sal_Int16 _nId,
        ::com::sun::star::uno::Sequence< ::rtl::OUString > _rInterceptedSchemes /// if not empty, this will be used for getInterceptedURLs
    );

    // StarOne
    DECLARE_UNO3_DEFAULTS(FmXDispatchInterceptorImpl, FmXDispatchInterceptorImpl_BASE);
    //  virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& type) throw ( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);
    //  ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);


    // ::com::sun::star::frame::XDispatchProvider
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL queryDispatch( const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > > SAL_CALL queryDispatches( const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& aDescripts ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::frame::XDispatchProviderInterceptor
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL getSlaveDispatchProvider(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setSlaveDispatchProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xNewDispatchProvider ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL getMasterDispatchProvider(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setMasterDispatchProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xNewSupplier ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::frame::XInterceptorInfo
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getInterceptedURLs(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    // OComponentHelper
    virtual void SAL_CALL disposing();

protected:
    void ImplDetach();

    ::osl::Mutex&       getAccessSafety()
    {
        if (m_pMaster && m_pMaster->getInterceptorMutex())
            return *m_pMaster->getInterceptorMutex();
        return m_aFallback;
    }
};

//==================================================================
// Stringkonvertierung
//==================================================================
::rtl::OUString AnyToStr( const ::com::sun::star::uno::Any& aValue);
::com::sun::star::uno::Any StringToAny( ::rtl::OUString aStr, ::com::sun::star::uno::TypeClass eTargetType );
::com::sun::star::uno::Sequence< ::rtl::OUString> getEventMethods(const ::com::sun::star::uno::Type& type);

//==================================================================
// ...
//==================================================================
//sal_Int16 getControlTypeByModelName(const ::rtl::OUString& rModel);
//::rtl::OUString   getModelNameByControlType(sal_Int16 nType);
::rtl::OUString     getServiceNameByControlType(sal_Int16 nType);
    // get a service name to create a model of the given type (OBJ_FM_...)
sal_Int16       getControlTypeByObject(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XServiceInfo>& _rxObject);
    // get the object type (OBJ_FM_...) from the services the object supports

//  void TransferFormComponentProperties(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& xOld, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& xNew);
void TransferEventScripts(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel>& xModel, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& xControl,
    const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor>& rTransferIfAvailable);

sal_Int16   GridModel2ViewPos(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& rColumns, sal_Int16 nModelPos);
sal_Int16   GridView2ModelPos(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& rColumns, sal_Int16 nViewPos);
sal_Int16   GridViewColumnCount(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& rColumns);

//==================================================================
UniString quoteName(const UniString& rQuote, const UniString& rName);
UniString quoteTableName(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& xMeta, const UniString& rName);

//==================================================================
//::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> getTableFields(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rxConn, const ::rtl::OUString& _rsName);
sal_Bool isLoadable(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xLoad);
sal_Bool isRowSetAlive(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _rxRowSet);
    // checks if the ::com::sun::star::sdbcx::XColumnsSupplier provided by _rxRowSet supllies any columns


//==================================================================
//= a class wrapping an object implementing a sdb::DataColumn service
//==================================================================
class DataColumn
{
    // interfaces needed for sddb::Column
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>                m_xPropertySet;
    // interfaces needed for sdb::DataColumn
    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn>                   m_xColumn;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumnUpdate>         m_xColumnUpdate;

public:
    DataColumn() { };
    DataColumn(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxIFace);
    // if the object behind _rxIFace doesn't fully support the DataColumn service,
    // (which is checked via the supported interfaces) _all_ members will be set to
    // void !, even if the object has some of the needed interfaces.

    sal_Bool is() const { return m_xColumn.is(); }
    sal_Bool Is() const { return m_xColumn.is(); }
    sal_Bool supportsUpdate() const { return m_xColumnUpdate.is(); }

    DataColumn* operator ->() { return this; }
    //  operator ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> () const { return ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> m_xColumn; }
    operator ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> () const{ return m_xColumn;; }

    // 'conversions'
    operator const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>&() const                { return m_xPropertySet; }
    operator const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn>&() const                   { return m_xColumn; }
    operator const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumnUpdate>&() const
    {
        DBG_ASSERT(m_xColumnUpdate.is() , "operator XColumnUpdate(): is NULL!");
        return m_xColumnUpdate;
    }

    // das normale queryInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& type) throw ( ::com::sun::star::uno::RuntimeException )
    { return m_xColumn->queryInterface(type); }

    // ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>
    inline ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo> getPropertySetInfo() const throw( ::com::sun::star::uno::RuntimeException );
    inline void setPropertyValue(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue) throw( ::com::sun::star::beans::UnknownPropertyException,  ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    inline ::com::sun::star::uno::Any getPropertyValue(const ::rtl::OUString& PropertyName) const throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    inline void addPropertyChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener>& xListener) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    inline void removePropertyChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener>& aListener) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    inline void addVetoableChangeListener(const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener>& aListener) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    inline void removeVetoableChangeListener(const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener>& aListener) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::sdb::XColumn
    inline sal_Bool wasNull() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline ::rtl::OUString getString() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline sal_Bool getBoolean() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline sal_Int8 getByte() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline sal_Int16 getShort() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline sal_Int32 getInt() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline sal_Int64 getLong() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline float getFloat() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline double getDouble() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline ::com::sun::star::uno::Sequence< sal_Int8 > getBytes() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline ::com::sun::star::util::Date getDate() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline ::com::sun::star::util::Time getTime() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline ::com::sun::star::util::DateTime  getTimestamp() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream> getBinaryStream() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream> getCharacterStream() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline ::com::sun::star::uno::Any getObject(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& typeMap) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef> getRef() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob> getBlob() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob> getClob() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray> getArray() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );

    // XColumnUpdate
    inline void updateNull(void) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline void updateBoolean(sal_Bool x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline void updateByte(sal_Int8 x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline void updateShort(sal_Int16 x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline void updateInt(sal_Int32 x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline void updateLong(sal_Int64 x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline void updateFloat(float x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline void updateDouble(double x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline void updateString(const ::rtl::OUString& x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline void updateBytes(const ::com::sun::star::uno::Sequence< sal_Int8 >& x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline void updateDate(const com::sun::star::util::Date& x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline void updateTime(const ::com::sun::star::util::Time& x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline void updateTimestamp(const ::com::sun::star::util::DateTime& x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline void updateBinaryStream(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream>& x, sal_Int32 length) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline void updateCharacterStream(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream>& x, sal_Int32 length) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline void updateObject(const ::com::sun::star::uno::Any& x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    inline void updateNumericObject(const ::com::sun::star::uno::Any& x, sal_Int32 scale) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
};

// ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo> DataColumn::getPropertySetInfo() const throw( ::com::sun::star::uno::RuntimeException )
{
    return m_xPropertySet->getPropertySetInfo();
}

void DataColumn::setPropertyValue(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue) throw( ::com::sun::star::beans::UnknownPropertyException,  ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException )
{
    m_xPropertySet->setPropertyValue(aPropertyName, aValue);
}

::com::sun::star::uno::Any DataColumn::getPropertyValue(const ::rtl::OUString& PropertyName) const throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException )
{
    return m_xPropertySet->getPropertyValue(PropertyName);
}

void DataColumn::addPropertyChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener>& xListener) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException )
{
    m_xPropertySet->addPropertyChangeListener(aPropertyName, xListener);
}

void DataColumn::removePropertyChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener>& aListener) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException )
{
    m_xPropertySet->removePropertyChangeListener(aPropertyName, aListener);
}

void DataColumn::addVetoableChangeListener(const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener>& aListener) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException )
{
    m_xPropertySet->addVetoableChangeListener(PropertyName, aListener);
}

void DataColumn::removeVetoableChangeListener(const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener>& aListener) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException )
{
    m_xPropertySet->removeVetoableChangeListener(PropertyName, aListener);
}

// ::com::sun::star::sdb::XColumn
sal_Bool DataColumn::wasNull() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    return m_xColumn->wasNull();
}

::rtl::OUString DataColumn::getString() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    return m_xColumn->getString();
}

sal_Bool DataColumn::getBoolean() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    return m_xColumn->getBoolean();
}

sal_Int8 DataColumn::getByte() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    return m_xColumn->getByte();
}

sal_Int16 DataColumn::getShort() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    return m_xColumn->getShort();
}

sal_Int32 DataColumn::getInt() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    return m_xColumn->getInt();
}

sal_Int64 DataColumn::getLong() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    return m_xColumn->getLong();
}

float DataColumn::getFloat() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    return m_xColumn->getFloat();
}

double DataColumn::getDouble() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    return m_xColumn->getDouble();
}

::com::sun::star::uno::Sequence< sal_Int8 > DataColumn::getBytes() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    return m_xColumn->getBytes();
}

::com::sun::star::util::Date DataColumn::getDate() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    return m_xColumn->getDate();
}

::com::sun::star::util::Time DataColumn::getTime() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    return m_xColumn->getTime();
}

::com::sun::star::util::DateTime DataColumn::getTimestamp() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    return m_xColumn->getTimestamp();
}

::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream> DataColumn::getBinaryStream() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    return m_xColumn->getBinaryStream();
}

::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream> DataColumn::getCharacterStream() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    return m_xColumn->getCharacterStream();
}

::com::sun::star::uno::Any DataColumn::getObject(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& typeMap) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    return m_xColumn->getObject(typeMap);
}

::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef> DataColumn::getRef() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    return m_xColumn->getRef();
}

::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob> DataColumn::getBlob() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    return m_xColumn->getBlob();
}

::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob> DataColumn::getClob() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    return m_xColumn->getClob();
}

::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray> DataColumn::getArray() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    return m_xColumn->getArray();
}

// XColumnUpdate
void DataColumn::updateNull() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    m_xColumnUpdate->updateNull();
}

void DataColumn::updateBoolean(sal_Bool x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    m_xColumnUpdate->updateBoolean(x);
}

void DataColumn::updateByte(sal_Int8 x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    m_xColumnUpdate->updateByte(x);
}

void DataColumn::updateShort(sal_Int16 x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    m_xColumnUpdate->updateShort(x);
}

void DataColumn::updateInt(sal_Int32 x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    m_xColumnUpdate->updateInt(x);
}

void DataColumn::updateLong(sal_Int64 x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    m_xColumnUpdate->updateLong(x);
}

void DataColumn::updateFloat(float x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    m_xColumnUpdate->updateFloat(x);
}

void DataColumn::updateDouble(double x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    m_xColumnUpdate->updateDouble(x);
}

void DataColumn::updateString(const ::rtl::OUString& x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    m_xColumnUpdate->updateString(x);
}

void DataColumn::updateBytes(const ::com::sun::star::uno::Sequence< sal_Int8 >& x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    m_xColumnUpdate->updateBytes(x);
}

void DataColumn::updateDate(const com::sun::star::util::Date& x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    m_xColumnUpdate->updateDate(x);
}

void DataColumn::updateTime(const ::com::sun::star::util::Time& x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    m_xColumnUpdate->updateTime(x);
}

void DataColumn::updateTimestamp(const ::com::sun::star::util::DateTime& x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    m_xColumnUpdate->updateTimestamp(x);
}

void DataColumn::updateBinaryStream(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream>& x, sal_Int32 length) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    m_xColumnUpdate->updateBinaryStream(x, length);
}

void DataColumn::updateCharacterStream(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream>& x, sal_Int32 length) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    m_xColumnUpdate->updateCharacterStream(x, length);
}

void DataColumn::updateObject(const ::com::sun::star::uno::Any& x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    m_xColumnUpdate->updateObject(x);
}

void DataColumn::updateNumericObject(const ::com::sun::star::uno::Any& x, sal_Int32 scale) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    m_xColumnUpdate->updateNumericObject(x, scale);
}



#endif // _SVX_FMTOOLS_HXX

