/*************************************************************************
 *
 *  $RCSfile: UITools.hxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:54:49 $
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
#ifndef DBAUI_TOOLS_HXX
#define DBAUI_TOOLS_HXX

#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef DBAUI_TYPEINFO_HXX
#include "TypeInfo.hxx"
#endif
#ifndef _SVX_SVXENUM_HXX
#include <svx/svxenum.hxx>
#endif
#ifndef _SV_TASKPANELIST_HXX
#include <vcl/taskpanelist.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif

#define RET_ALL     10

// we only need forward decl here
namespace com { namespace sun { namespace star {

    namespace beans     { class XPropertySet;}
    namespace container
    {
        class XNameAccess;
        class XHierarchicalNameContainer;
        class XNameContainer;
    }
    namespace lang
    {
        class XEventListener;
        class XMultiServiceFactory;
    }
    namespace awt
    {
        struct FontDescriptor;
    }
    namespace sdbc
    {
        class XDatabaseMetaData;
        class XConnection;
    }
    namespace util
    {
        struct URL;
        class XNumberFormatter;
    }
    namespace ucb { class XContent; }

}}}

namespace svt
{
    class EditBrowseBox;
}

class Window;
class ToolBox;
class Font;
class SvNumberFormatter;
class SvLBoxEntry;

// .........................................................................
namespace dbaui
{
// .........................................................................
    class IContainerFoundListener;
    class ODsnTypeCollection;
    class DBTreeListBox;

    /** creates a new connection and appends the eventlistener
        @param  _rsDataSourceName       name of the datasource
        @param  _xDatabaseContext       the database context
        @param  _rEvtLst                the eventlistener which will be added to the new created connection
        @param  _rOUTConnection         this parameter will be filled with the new created connection
        @return SQLExceptionInfo        contains a SQLException, SQLContext or a SQLWarning when they araised else .isValid() will return false
    */
    ::dbtools::SQLExceptionInfo createConnection(
                                    const ::rtl::OUString& _rsDataSourceName,
                                     const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xDatabaseContext,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rMF,
                                    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener>& _rEvtLst,
                                    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rOUTConnection );
    /** creates a new connection and appends the eventlistener
        @param  _xDataSource            the datasource
        @param  _rEvtLst                the eventlistener which will be added to the new created connection
        @param  _rOUTConnection         this parameter will be filled with the new created connection
        @return SQLExceptionInfo        contains a SQLException, SQLContext or a SQLWarning when they araised else .isValid() will return false
    */
    ::dbtools::SQLExceptionInfo createConnection(
                                     const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _xDataSource,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rMF,
                                    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener>& _rEvtLst,
                                    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rOUTConnection );

    /**  creates a error dialog which displays the SQLExceptionInfo. Also it supports a "more" button where detailed information are available
        @param  _rInfo                  the error which should be shown, if the info is not valid no error dialog will appear
        @param  _pParent                the parent of the error dialog
        @param  _xFactory               need to create the dialog
    */
    void showError( const ::dbtools::SQLExceptionInfo& _rInfo,
                    Window* _pParent,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xFactory);

    /** return a vector which contains all key columns for the @see com::sun::star::sdbc::KeyType _nKeyType
        @param  _rxTable                the table which must be a @see com::sun::star::sdbcx::XColumnsSupplier
        @param  _nKeyType               @see com::sun::star::sdbc::KeyType
    */

    ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> >
        getKeyColumns(  const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxTable,
                        sal_Int32 _nKeyType);

    /** fills a map and a vector with localized type names
        @param  _rxConnection   the connection to acces the metadata
        @param  _rsTypeNames    a list of localized type names seperated with ';'
        @param  _rTypeInfoMap   the filled map with the type names
        @param  _rTypeInfoIters the vector filled with map iterators
    */
    void fillTypeInfo(  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rxConnection,
                        const String& _rsTypeNames,
                        OTypeInfoMap& _rTypeInfoMap,
                        ::std::vector<OTypeInfoMap::iterator>& _rTypeInfoIters);

    /** fill a column with data of a field description
        @param  _rxColumn   the column which should be filled
        @param  _pFieldDesc the source of the data
    */
    class OFieldDescription;
    void setColumnProperties(   const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxColumn,
                                const OFieldDescription* _pFieldDesc);

    ::rtl::OUString createDefaultName(  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& _xMetaData,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _xTables,
                                        const ::rtl::OUString& _sName);

    /** checks if the given name exists in the database context
    */
    sal_Bool checkDataSourceAvailable(  const ::rtl::OUString& _sDataSourceName,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xFactory_xORB);

    /** maps SvxCellHorJustify to com::sun::star::awt::TextAlign
        @param SvxCellHorJustify& _eAlignment
        @return the corresponding com::sun::star::awt::TextAlign
    */
    sal_Int32 mapTextAllign(const SvxCellHorJustify& _eAlignment);

    /** maps com::sun::star::awt::TextAlign to SvxCellHorJustify
        @param com::sun::star::awt::TextAlign& _nAlignment
        @return the corresponding SvxCellHorJustify
    */
    SvxCellHorJustify mapTextJustify(const sal_Int32& _nAlignment);

    /** fill a column with ui data of a field description
        @param  _rxColumn   the column which should be filled
        @param  _pFieldDesc the source of the data
    */
    void setColumnUiProperties( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxColumn,
                                const OFieldDescription* _pFieldDesc);

    /** convert Font to ::com::sun::star::awt::FontDescriptor
        @param  _rFont  Font to be converted
        @return the new FontDescriptor
    */
    ::com::sun::star::awt::FontDescriptor CreateFontDescriptor( const Font& _rFont );

    /** call teh format dialog and set the selected format at the column
        @param  _xAffectedCol   Font to be converted
        @param  _xField         Font to be converted
    */
    void callColumnFormatDialog(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xAffectedCol,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xField,
                                SvNumberFormatter* _pFormatter,
                                Window* _pParent);

    /** second variant of the function before
    */
    sal_Bool callColumnFormatDialog(Window* _pParent,
                                    SvNumberFormatter* _pFormatter,
                                    sal_Int32 _nDataType,
                                    sal_Int32& _nFormatKey,
                                    SvxCellHorJustify& _eJustify,
                                    sal_uInt16& _nFlags,
                                    sal_Bool  _bHasFormat);
    /** append a name to tablefilter of a datasource
        @param  _xConnection    the connection is need to get the datasource
        @param  _sName          the name which should be appended
        @param  _xFactory       needed to check if datasource is available
        @param  _pParent        needed when an error must be shown
        @return false when datsource is not available otherwise true
    */
    sal_Bool appendToFilter(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection,
                            const ::rtl::OUString& _sName,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xFactory,
                            Window* _pParent);

    /** notifySystemWindow adds or remove the given window _pToRegister at the Systemwindow found when search _pWindow.
        @param  _pWindow
            The window which is used to search for the SystemWindow.
        @param  _pToRegister
            The window which should be added or removed on the TaskPaneList.
        @param  _rMemFunc
            The member function which should be called at the SystemWindow when found.
            Possible values are:
            ::comphelper::mem_fun(&TaskPaneList::AddWindow)
            ::comphelper::mem_fun(&TaskPaneList::RemoveWindow)
    */
    void notifySystemWindow(Window* _pWindow,
                            Window* _pToRegister,
                            ::comphelper::mem_fun1_t<TaskPaneList,Window*> _rMemFunc);

    /** adjustToolBoxSize checks if the size of the ToolBox is still valid. If not it will be resized.
        @param  _pToolBox
            The Toolbox which should be resized.
    */
    void adjustToolBoxSize(ToolBox* _pToolBox);

    /** isHiContrast check if we are in hi contrast mode.
        @param  _pWindow
            The window we have to check on.
        @return
            <TRUE/> if so, otherwise <FALSE/>
    */
    sal_Bool isHiContrast(Window* _pWindow);

    void adjustBrowseBoxColumnWidth( ::svt::EditBrowseBox* _pBox, sal_uInt16 _nColId );

    /** check if SQL92 name checking is enabled
        @param  _xConnection
            Used to get the datasource as parent from the connection.
        @return
            <TRUE/> if so otherwise <FALSE/>
    */
    sal_Bool isSQL92CheckEnabled(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection);

    /** check if the alias name of the table should be added at select statements
        @param  _xConnection
            Used to get the datasource as parent from the connection.
        @return
            <TRUE/> if so otherwise <FALSE/>
    */
    sal_Bool isAppendTableAliasEnabled(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection);

    /** fills the bool and string value with information out of the datasource info property
        @param  _xDatasource
            Asked for the properties.
        @param  _rAutoIncrementValueEnabled
            <OUT/> Set to TRUE when the property was set in the datasource.
        @param  _rsAutoIncrementValue
            <OUT/> Set to the value when the property was set in the datasource.
    */
    void fillAutoIncrementValue(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xDatasource
                                ,sal_Bool& _rAutoIncrementValueEnabled
                                ,::rtl::OUString& _rsAutoIncrementValue);

    /** fills the bool and string value with information out of the datasource info property
        @param  _xConnection
            Used to get the datasource as parent from the connection.
        @param  _rAutoIncrementValueEnabled
            <OUT/> Set to TRUE when the property was set in the datasource.
        @param  _rsAutoIncrementValue
            <OUT/> Set to the value when the property was set in the datasource.
    */
    void fillAutoIncrementValue(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection
                                ,sal_Bool& _rAutoIncrementValueEnabled
                                ,::rtl::OUString& _rsAutoIncrementValue);

    /** creates the URL or the help agent
        @param  _sModuleName
        @param  _nHelpId
        @return
            The URL for the help agent to dispatch.
    */
    ::com::sun::star::util::URL createHelpAgentURL(const ::rtl::OUString& _sModuleName,const sal_Int32 _nHelpId);

    /** set the evaluation flag at the number formatter
        @param  _rxFormatter
    */
    void setEvalDateFormatForFormatter(::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter);

    /** query for a type info which can be used to create a primary key column
        @param  _rTypeInfo
            The map which contains all available types.
        @return
            The type info which can be used to create a primary key column.
    */
    TOTypeInfoSP queryPrimaryKeyType(const OTypeInfoMap& _rTypeInfo);

    /** query for a specific type.
        @param  _nDataType
            The type we are searching.
        @param  _rTypeInfo
            The map which contains all available types.
        @return
            The type or <NULL/> if we can't find it.
    */
    TOTypeInfoSP queryTypeInfoByType(sal_Int32 _nDataType,const OTypeInfoMap& _rTypeInfo);

    /** returns the configuration node name of user defined drivers.
        @return
            the configuration node name of user defined drivers.
    */
    ::rtl::OUString getUserDefinedDriverNodeName();

    /** returns the configuration node name of user defined driver type display name.
        @return
            the configuration node name of user defined driver type display name.
    */
    ::rtl::OUString getDriverTypeDisplayNodeName();

    /** returns the configuration node name of user defined driver DSN prefix.
        @return
            the configuration node name of user defined driver DSN prefix.
    */
    ::rtl::OUString getDriverDsnPrefixNodeName();

    /** returns the result of the user action when view the query dialog.
        @param  _pParent
            The parent of the dialog
        @param  _nTitle
            A string resource id for the text which will be displayed as title.
        @param  _nText
            A string resource id for the text which will be displayed above the buttons.
            When the string contains a #1. This will be replaced by the name.
        @param  _bAll
            When set to <TRUE/>, the all button will be appended.
        @param  _sName
            The name of the object to ask for.
        @return
            RET_YES, RET_NO, RET_ALL
    */
    sal_Int32 askForUserAction(Window* _pParent,USHORT _nTitle,USHORT _nText,sal_Bool _bAll,const ::rtl::OUString& _sName);

    /** creates a new view from a query or table
        @param  _sName
            The name of the view to be created.
        @param  _xConnection
            The source connection.
        @param  _xSourceObject
            The object for which a view should be created.
        @return
            The created view.
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> createView( const ::rtl::OUString& _sName
                                                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection
                                                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xSourceObject);

    /** returns the stripped database name.
        @param  _xDataSource
            The data source
        @param  _rsDatabaseName
            Will be filled with the original data source if it is empty.
        @return
            The stripped database name either the registered naem or if it is a file url the last segment.
    */
    ::rtl::OUString getStrippedDatabaseName(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xDataSource
                                            ,::rtl::OUString& _rsDatabaseName);

    /** converts the URL for UI purpose, system file based URLs will be converted into system path.
        @param  _bPrefix
            <TRUE/> if the type prefix should append in front of the URL, otherwise it will be dropped.
        @param  _pCollection
            The type collection.
        @param  _sURL
            The URL which should be converted.
        @return
            The new converted URL.
    */
    String convertURLtoUI(sal_Bool _bPrefix,ODsnTypeCollection* _pCollection,const ::rtl::OUString& _sURL);

    /** fills the tree list box with the elements from the given container and sub elements.
        @param  _xContainer
            The container.
        @param  _rList
            The list to fill.
        @param  _nImageId
            The image id for the leaf elements which do not support a XNameAccess
        @param  _pParent
            The root entry.
        @param  _pContainerFoundListener
            Will be called if a sub container was found
    */
    void fillTreeListNames(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xContainer
                        ,DBTreeListBox& _rList,USHORT _nImageId,SvLBoxEntry* _pParent = NULL,IContainerFoundListener* _pContainerFoundListener = NULL);

    /** opens a save dialog to store a form or report folder in the current hierachy.
        @param  _pParent
            The parent of the dialog.
        @param  _xNames
            Where to insert the new object.
        @param  _sParentFolder
            The name of the parent folder.
        @param  _bForm
            <TRUE/> if a form should be inserted
        @param  _bCollection
            A folder should be inserted
        @param  _xContent
            The content which should be copied.
        @param  _bMove
                if <TRUE/> the name of the content must be inserted without any change, otherwise not.
        @return
            <TRUE/> if the insert opertions was successfull, otherwise <FALSE/>.
    */
    sal_Bool insertHierachyElement(Window* _pParent
                           ,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XHierarchicalNameContainer>& _xNames
                           ,const String& _sParentFolder
                           ,sal_Bool _bForm
                           ,sal_Bool _bCollection = sal_True
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent>& _xContent = ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent>()
                            ,sal_Bool _bMove = sal_False);

    /** deletes the objects in the name container. (forms or reports or queries)
        @param  _pParent
            The parent window.
        @param  _xFactory
            The factory
        @param  _xNames
            The container to remove from
        @param  _rList
            The list of names.
        @param  _nTextResource
            The text string to be shown.
        @param  _bConfirm
            If <TRUE/> a dialog appears to confirm the delete operation, otherwise not.
    */
    void deleteObjects(Window* _pParent
                   ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xFactory
                   ,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer>& _xNames
                   ,const ::std::vector< ::rtl::OUString>& _rList
                   ,sal_uInt16 _nTextResource
                   ,sal_Bool _bConfirm = sal_True);
// .........................................................................
}
// .........................................................................
#endif // DBAUI_TOOLS_HXX


