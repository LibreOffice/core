/*************************************************************************
 *
 *  $RCSfile: datman.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2000-11-13 11:41:26 $
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

#ifndef _BIB_DATMAN_HXX
#define _BIB_DATMAN_HXX

#ifndef _COM_SUN_STAR_AWT_XCONTROLMODEL_HPP_
#include <com/sun/star/awt/XControlModel.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSER_HPP_
#include <com/sun/star/sdb/XSQLQueryComposer.hpp>
#endif

#ifndef _COM_SUN_STAR_REGISTRY_XSIMPLEREGISTRY_HPP_
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx> // helper for implementations
#endif

class BibRegistry;
class Window;

//-----------------------------------------------------------------------------
class BibView;
class BibToolBar;
class BibGridwin;
struct BibDBDescriptor;
class BibDataManager :  public cppu::WeakImplHelper1 < ::com::sun::star::beans::XPropertyChangeListener>
{
private:
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >                   xForm;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >            xGridModel;
        ::com::sun::star::uno::Reference< ::com::sun::star::registry::XSimpleRegistry >             xRegistry;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >               xSourceProps;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >               xGlobalProps;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer >        xParser;
        ::rtl::OUString                     aActiveDataTable;
        ::rtl::OUString                     aDataSourceURL;
        ::rtl::OUString                     aQuoteChar;
//      sal_Bool                        bNew;
//      sal_Bool                        bModified;
        ::com::sun::star::uno::Any                      aUID;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >              xBibCursor;

        BibView*                    pBibView;
        BibToolBar*                 pToolbar;
        BibGridwin*                 pGridWin;

        rtl::OUString               sIdentifierMapping;
protected:

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >               createGlobalProperties();
        void                        InsertFields(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent > & xGrid);
        void                        InitRegistry();
        void                        SetMeAsUidListener();
        void                        RemoveMeAsUidListener();

        void                        UpdateAddressbookCursor(::rtl::OUString aSourceName);
public:

        BibDataManager(BibRegistry * pRegistry);
        ~BibDataManager();

        virtual void                SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt)
                                                                throw( ::com::sun::star::uno::RuntimeException );
        virtual void                SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source );



        ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >                   createDatabaseForm( BibDBDescriptor&    aDesc);
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >                   getDatabaseForm();

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >            createGridModel(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm > & xDbForm);
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >            createGridModel();

        void                        loadDatabase();
        void                        unloadDatabase();

        ::com::sun::star::uno::Sequence< ::rtl::OUString>           getDataSources();

        ::rtl::OUString             getActiveDataSource() {return aDataSourceURL;}
        void                        setActiveDataSource(const ::rtl::OUString& rURL);

        ::rtl::OUString             getActiveDataTable();
        void                        setActiveDataTable(const ::rtl::OUString& rTable);

        void                        setFilter(const ::rtl::OUString& rQuery);
        ::rtl::OUString                     getFilter();

        ::com::sun::star::uno::Sequence< ::rtl::OUString>           getQueryFields();
        void                        setQueryField(const ::rtl::OUString& rField);
        ::rtl::OUString                     getQueryField();
        void                        startQueryWith(const ::rtl::OUString& rQuery);
        ::rtl::OUString                     getQueryString();
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer >        getParser();

//      void                        saveGridModel(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > & xDbForm);
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >            loadGridModel(const ::rtl::OUString& rName);

        void                        setBeamerSize(long nSize);
        long                        getBeamerSize();
        void                        setViewSize(long nSize);
        long                        getViewSize();

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >               getViewProperties();

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >               getChildViewProperties(const ::rtl::OUString& rName);

        ::rtl::OUString                     getControlName(sal_Int32 nFormatKey );

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >            loadControlModel(const ::rtl::OUString& rName,
                                                        sal_Bool bForceListBox = sal_False);
        void                        saveCtrModel(const ::rtl::OUString& rName,
                                                    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > & rCtrModel);

//      sal_Bool                        isNew() { return bNew;}
//      sal_Bool                        isModified() { return bModified;}

        sal_Bool                        moveRelative(long nMove);

        void                        CreateMappingDialog(Window* pParent);
        ::rtl::OUString                     CreateDBChangeDialog(Window* pParent);

        void                        SetView(BibView* pView) {pBibView = pView;}

        void                        SetToolbar(BibToolBar* pSet) {pToolbar = pSet;}
        void                        SetGridWin(BibGridwin* pSet) {pGridWin = pSet;}

        const rtl::OUString&        GetIdentifierMapping();
        void                        ResetIdentifierMapping() {sIdentifierMapping = rtl::OUString();}
};


#endif
