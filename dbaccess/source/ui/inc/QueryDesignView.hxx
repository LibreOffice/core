/*************************************************************************
 *
 *  $RCSfile: QueryDesignView.hxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 17:52:36 $
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
#ifndef DBAUI_QUERYDESIGNVIEW_HXX
#define DBAUI_QUERYDESIGNVIEW_HXX

#ifndef DBAUI_QUERYVIEW_HXX
#include "queryview.hxx"
#endif
#ifndef _SV_SPLIT_HXX
#include <vcl/split.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef DBAUI_ENUMTYPES_HXX
#include "QEnumTypes.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef DBAUI_QUERYCONTROLLER_HXX
#include "querycontroller.hxx"
#endif
#ifndef DBAUI_CONNECTIONLINEDATA_HXX
#include "ConnectionLineData.hxx"
#endif

namespace connectivity
{
    class OSQLParseNode;
}

class ComboBox;
namespace dbaui
{
    enum SqlParseError
    {
        eIllegalJoin,
        eStatementTooLong,
        eNoConnection,
        eNoSelectStatement,
        eStatementTooComplex,
        eColumnInLikeNotFound,
        eNoColumnInLike,
        eColumnNotFound,
        eNativeMode,
        eTooManyTables,
        eTooManyConditions,
        eTooManyColumns,
        eIllegalJoinCondition,
        eOk
    };

    class OQueryViewSwitch;
    class OAddTableDlg;
    class OQueryTableWindow;
    class OSelectionBrowseBox;
    class OTableConnection;
    class OQueryTableConnectionData;
    class OQueryContainerWindow;

    class OQueryDesignView : public OQueryView
    {
        enum ChildFocusState
        {
            SELECTION,
            TABLEVIEW,
            NONE
        };

        Splitter                            m_aSplitter;

        ::com::sun::star::lang::Locale      m_aLocale;
        ::rtl::OUString                     m_sDecimalSep;

        OSelectionBrowseBox*                m_pSelectionBox;    // presents the lower window
        ChildFocusState                     m_eChildFocus;
        sal_Bool                            m_bInKeyEvent;
        sal_Bool                            m_bInSplitHandler;

    public:
        OQueryDesignView(OQueryContainerWindow* pParent, OQueryController* _pController,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
        virtual ~OQueryDesignView();

        virtual sal_Bool isCutAllowed();
        virtual sal_Bool isPasteAllowed();
        virtual sal_Bool isCopyAllowed();
        virtual void copy();
        virtual void cut();
        virtual void paste();
        // clears the whole query
        virtual void clear();
        // set the view readonly or not
        virtual void setReadOnly(sal_Bool _bReadOnly);
        // check if the statement is correct when not returning false
        virtual sal_Bool checkStatement();
        // set the statement for representation
        virtual void setStatement(const ::rtl::OUString& _rsStatement);
        // returns the current sql statement
        virtual ::rtl::OUString getStatement();
        /// late construction
        virtual void Construct();
        virtual void initialize();
        // window overloads
        virtual long PreNotify( NotifyEvent& rNEvt );
        virtual void GetFocus();

        BOOL IsAddAllowed();
        sal_Bool isSlotEnabled(sal_Int32 _nSlotId);
        void setSlotEnabled(sal_Int32 _nSlotId,sal_Bool _bEnable);
        void setNoneVisbleRow(sal_Int32 _nRows);

        ::com::sun::star::lang::Locale      getLocale() const           { return m_aLocale;}
        ::rtl::OUString                     getDecimalSeparator() const { return m_sDecimalSep;}

        sal_Bool HasTable() const;
        SqlParseError InsertField( const OTableFieldDescRef& rInfo, sal_Bool bVis=sal_True, sal_Bool bActivate = sal_True);
        // save the position of the table window and the pos of the splitters
        void SaveTabWinUIConfig(OQueryTableWindow* pWin);
        // called when fields are deleted
        void DeleteFields( const ::rtl::OUString& rAliasName );
        // called when a table from tabeview was deleted
        void TableDeleted(const ::rtl::OUString& rAliasName);

        BOOL getColWidth( const ::rtl::OUString& rAliasName, const ::rtl::OUString& rFieldName, ULONG& nWidth );
        void fillValidFields(const ::rtl::OUString& strTableName, ComboBox* pFieldList);
        void zoomTableView(const Fraction& _rFraction);

        void SaveUIConfig();
        void stopTimer();
        void startTimer();
        void reset();
        sal_Bool InitFromParseNode();

        ::connectivity::OSQLParseNode* getPredicateTreeFromEntry(   OTableFieldDescRef pEntry,
                                                                    const String& _sCriteria,
                                                                    ::rtl::OUString& _rsErrorMessage,
                                                                    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxColumn);

    protected:
        // return the Rectangle where I can paint myself
        virtual void resizeDocumentView(Rectangle& rRect);
        DECL_LINK( SplitHdl, void* );
    };
}
#endif // DBAUI_QUERYDESIGNVIEW_HXX

