/*************************************************************************
 *
 *  $RCSfile: chart2uno.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $  $Date: 2004-02-11 09:55:41 $
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

#ifndef SC_CHART2UNO_HXX
#define SC_CHART2UNO_HXX

#ifndef SC_RANGELST_HXX
#include "rangelst.hxx"
#endif
#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDATAPROVIDER_HPP_
#include <com/sun/star/chart2/XDataProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDATASOURCE_HPP_
#include <com/sun/star/chart2/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDATASEQUENCE_HPP_
#include <com/sun/star/chart2/XDataSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATASEQUENCEROLE_HPP_
#include <com/sun/star/chart2/DataSequenceRole.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

class ScDocShell;


// DataProvider ==============================================================

class ScChart2DataProvider : public
                ::cppu::WeakImplHelper2<
                    ::com::sun::star::chart2::XDataProvider,
                    ::com::sun::star::lang::XServiceInfo>,
                SfxListener
{
public:

    explicit ScChart2DataProvider( ScDocShell* pDocSh);
    virtual ~ScChart2DataProvider();
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    // XDataProvider ---------------------------------------------------------

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XDataSource> SAL_CALL
        getDataByRangeRepresentation(
                const ::rtl::OUString& rRangeRepresentation)
        throw( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XDataSequence> SAL_CALL
        getDataSequenceByRangeIdentifier(
                const ::rtl::OUString& rRangeIdentifier)
        throw( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XDataSequence> SAL_CALL replaceRange(
                const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSequence>& rSeq)
        throw( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addDataChangeListener(
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataChangeListener>& rListener,
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSource>& rData)
        throw( ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removeDataChangeListener(
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataChangeListener>& rListener,
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSource>& rData)
        throw( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException);

    // XServiceInfo ----------------------------------------------------------

    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(
            ::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString&
            rServiceName) throw( ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
        getSupportedServiceNames() throw(
                ::com::sun::star::uno::RuntimeException);

private:

    ScDocShell*                 pDocShell;

};


// DataSource ================================================================

class ScChart2DataSource : public
                ::cppu::WeakImplHelper2<
                    ::com::sun::star::chart2::XDataSource,
                    ::com::sun::star::lang::XServiceInfo>,
                SfxListener
{
public:

    explicit ScChart2DataSource( ScDocShell* pDocSh, const ScRangeListRef&
            rRangeList);
    virtual ~ScChart2DataSource();
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    // XDataSource -----------------------------------------------------------

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XDataSequence> > SAL_CALL
        getDataSequences() throw( ::com::sun::star::uno::RuntimeException);

    // XServiceInfo ----------------------------------------------------------

    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(
            ::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString&
            rServiceName) throw( ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
        getSupportedServiceNames() throw(
                ::com::sun::star::uno::RuntimeException);

private:

    ScRangeListRef              xRanges;
    ScDocShell*                 pDocShell;

};


// DataSequence ==============================================================

class ScChart2DataSequence : public
                ::cppu::WeakImplHelper3<
                    ::com::sun::star::chart2::XDataSequence,
                    ::com::sun::star::beans::XPropertySet,
                    ::com::sun::star::lang::XServiceInfo>,
                SfxListener
{
public:

    explicit ScChart2DataSequence( ScDocShell* pDocSh,
            const ScRangeListRef& rRangeList );
    virtual ~ScChart2DataSequence();
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    // XDataSequence ---------------------------------------------------------

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>
        SAL_CALL getData() throw( ::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getSourceIdentifier() throw (
            ::com::sun::star::uno::RuntimeException);

    // XPropertySet ----------------------------------------------------------

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySetInfo> SAL_CALL
        getPropertySetInfo() throw( ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setPropertyValue(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Any& rValue)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
            const ::rtl::OUString& rPropertyName)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addPropertyChangeListener(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertyChangeListener>& xListener)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removePropertyChangeListener(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertyChangeListener>& rListener)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addVetoableChangeListener(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XVetoableChangeListener>& rListener)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removeVetoableChangeListener(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XVetoableChangeListener>& rListener)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    // XServiceInfo ----------------------------------------------------------

    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(
            ::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString&
            rServiceName) throw( ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
        getSupportedServiceNames() throw(
                ::com::sun::star::uno::RuntimeException);

private:

    // properties
    ::com::sun::star::chart2::DataSequenceRole  aRole;
    sal_Bool                    bHidden;
    // internals
    ScRangeListRef              xRanges;
    ::rtl::OUString             aIdentifier;
    ScDocShell*                 pDocShell;

};

#endif // SC_CHART2UNO_HXX
