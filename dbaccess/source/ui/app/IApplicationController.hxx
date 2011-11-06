/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef DBA30D_IAPPLICATIONCONTROLLER_HXX
#define DBA30D_IAPPLICATIONCONTROLLER_HXX

#include "IController.hxx"
#include "AppElementType.hxx"
#include "callbacks.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

class SvTreeListBox;
class SvLBoxEntry;

//........................................................................
namespace dbaui
{
//........................................................................

    class SAL_NO_VTABLE IApplicationController  :public IController
                                                ,public IControlActionListener
                                                ,public IContextMenuProvider
    {
    public:
        /** called when an entry in a tree list box has been double-clicked
            @param  _rTree
                The tree list box.
            @return
                <TRUE/> if the double click event has been handled by the called, and should not
                be processed further.
        */
        virtual bool onEntryDoubleClick( SvTreeListBox& _rTree ) = 0;

        /** called when a container (category) in the application view has been selected
            @param  _pTree
                The tree list box.
            @return
                <TRUE/> if the cotainer could be changed otherwise <FALSE/>
        */
        virtual sal_Bool onContainerSelect(ElementType _eType) = 0;

        /** called when an entry in a tree view has been selected
            @param  _pEntry
                the selected entry
        */
        virtual void onSelectionChanged() = 0;

        /** called when a "Cut" command is executed in a tree view
        */
        virtual void onCutEntry() = 0;

        /** called when a "Copy" command is executed in a tree view
        */
        virtual void onCopyEntry() = 0;

        /** called when a "Paste" command is executed in a tree view
        */
        virtual void onPasteEntry() = 0;

        /** called when a "Delete" command is executed in a tree view
        */
        virtual void onDeleteEntry() = 0;

        /// called when the preview mode was changed
        virtual void previewChanged( sal_Int32 _nMode ) = 0;

        /// called when an object container of any kind was found during enumerating tree view elements
        virtual void containerFound( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >& _xContainer) = 0;
    };

//........................................................................
} // namespace dbaui
//........................................................................

#endif // DBA30D_IAPPLICATIONCONTROLLER_HXX
