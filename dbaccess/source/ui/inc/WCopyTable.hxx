/*************************************************************************
 *
 *  $RCSfile: WCopyTable.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-23 14:59:07 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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

#ifndef DBAUI_WIZ_COPYTABLEDIALOG_HXX
#define DBAUI_WIZ_COPYTABLEDIALOG_HXX

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATA_HPP_
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef DBAUI_TYPEINFO_HXX
#include "TypeInfo.hxx"
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SVT_WIZDLG_HXX
#include <svtools/wizdlg.hxx>
#endif
#ifndef DBAUI_DATABASEEXPORT_HXX
#include "DExport.hxx"
#endif
#ifndef DBAUI_WIZ_TABBPAGE_HXX
#include "WTabPage.hxx"
#endif
#ifndef DBAUI_FIELDDESCRIPTIONS_HXX
#include "FieldDescriptions.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XKEYSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#endif


namespace dbaui
{

    // ========================================================
    // Wizard Dialog
    // ========================================================
    class OCopyTableWizard : public WizardDialog
    {
        friend class        OWizColumnSelect;
        friend class        OWizTypeSelect;
        friend class        OWizTypeSelectControl;
        friend class        OCopyTable;
        friend class        OWizNameMatching;
    public:
        DECLARE_STL_MAP(::rtl::OUString,::rtl::OUString,::comphelper::UStringMixLess,TNameMapping);
    private:
        ODatabaseExport::TColumns       m_vDestColumns; // contains the columns
        ODatabaseExport::TColumnVector  m_aDestVec;     // the order to insert the columns
        ODatabaseExport::TColumns       m_vSourceColumns;
        ODatabaseExport::TColumnVector  m_vSourceVec;

        OKButton                m_pbFinish;
        CancelButton            m_pbCancel;
        PushButton              m_pbNext;
        PushButton              m_pbPrev;
        String                  m_sTypeNames;       // these type names are the ones out of the resource file
        sal_uInt32              m_nPageCount;

        DECL_LINK( ImplPrevHdl  , PushButton* );
        DECL_LINK( ImplNextHdl  , PushButton* );
        DECL_LINK( ImplOKHdl    , OKButton* );
        DECL_LINK( ImplActivateHdl, WizardDialog* );
        void CheckColumns();
        void fillTypeInfo();
        void loadData();
        void construct();
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> getKeyColumns() const;
        // need for table creation
        void appendColumns(::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier>& _rxColSup,const ODatabaseExport::TColumnVector* _pVec,sal_Bool _bKeyColumns=sal_False);
        void appendKey(::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XKeysSupplier>& _rxSup,const ODatabaseExport::TColumnVector* _pVec);
        void setColumnProperties(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxColumn,const OFieldDescription* _pFieldDesc);

    protected:
        OTypeInfoMap                m_aTypeInfo;
        ::std::vector<OTypeInfoMap::iterator> m_aTypeInfoIndex;
        TNameMapping                m_mNameMapping;

        ::std::vector<sal_Int32>    m_vColumnPos;
        ::std::vector<sal_Int32>    m_vColumnTypes;

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xDestObject; // can be a query or a table
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >         m_xConnection;  // dest conn

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xSourceObject;

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>     m_xSourceColumns;       // container
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >    m_xFormatter;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory> m_xFactory;
        ::com::sun::star::lang::Locale  m_aLocale;
        ::rtl::OUString                 m_sName;    // for a table the name is composed
        ::rtl::OUString                 m_aKeyName;
    public:
        enum Wizard_Create_Style
        {
            WIZARD_APPEND_DATA = 0,
            WIZARD_DEF_DATA,
            WIZARD_DEF,
            WIZARD_DEF_VIEW
        };

        enum Wizard_Button_Style
        {
            WIZARD_NEXT,
            WIZARD_PREV,
            WIZARD_FINISH
        };

    protected:
        Wizard_Create_Style m_eCreateStyle;
        Wizard_Button_Style m_ePressed;
        sal_Bool            m_bCreatePrimaryColumn;

    public:
        // used for copy tables or queries
        OCopyTableWizard(Window * pParent,
                         const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >&       _xSourceObject,
                         const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >&         _xConnection,
                         const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >&    _xFormatter,
                         const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM);

        // used for importing rtf/html sources
        OCopyTableWizard(Window * pParent,
                        const ::rtl::OUString& _rDefaultName,
                        const ODatabaseExport::TColumns& _rDestColumns,
                        const ODatabaseExport::TColumnVector& _rSourceColVec,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _xFormatter,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM);

        virtual ~OCopyTableWizard();

        virtual long        DeactivatePage();
        OKButton&           GetOKButton() { return m_pbFinish; }
        sal_Bool            WasButtonPressed() const { return m_ePressed; }
        void                EnableButton(Wizard_Button_Style eStyle,sal_Bool bEnable);
        void                AddWizardPage(OWizardPage* pPage); // Page wird von OCopyTableWizard gelöscht
        void                RemoveWizardPage(OWizardPage* pPage); // Page goes again to user
        void                CheckButtons(); // checks which button can be disabled, enabled
        Wizard_Create_Style GetCreateStyle() const { return m_eCreateStyle; }
        // returns a vector where the position of a column and if the column is in the selection
        // when not the value is CONTAINER_ENTRY_NOTFOUND == (sal_uInt32)-1
        ::std::vector<sal_Int32>    GetColumnPositions()    const { return m_vColumnPos; }
        ::std::vector<sal_Int32>    GetColumnTypes()        const { return m_vColumnTypes; }
        const TNameMapping*         GetNameMapping()        const { return &m_mNameMapping; }

        void insertColumn(sal_Int32 _nPos,OFieldDescription* _pField);

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > getDestObject() const { return m_xDestObject; }
        // return sal_True if I need to set the autoincrement value by myself otherwise sal_False
        sal_Bool SetAutoincrement() const;

        const OTypeInfo*    getTypeInfo(sal_Int32 _nPos) const { return m_aTypeInfoIndex[_nPos]->second; }
        const OTypeInfoMap* getTypeInfo() const { return &m_aTypeInfo; }
        ::com::sun::star::lang::Locale  GetLocale() const { return m_aLocale; }
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > GetFormatter() const { return m_xFormatter; }
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory> GetFactory() const { return m_xFactory; }

        const ODatabaseExport::TColumns*        getSourceColumns() const{ return &m_vSourceColumns; }
        const ODatabaseExport::TColumnVector*   getSrcVector() const    { return &m_vSourceVec; }
        ODatabaseExport::TColumns*              getDestColumns()        { return &m_vDestColumns; }
        const ODatabaseExport::TColumnVector*   getDestVector() const   { return &m_aDestVec; }
        ::rtl::OUString getName() const { return m_sName; }

        void clearDestColumns();

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createTable();
    };
}

#endif // DBAUI_WIZ_COPYTABLEDIALOG_HXX


