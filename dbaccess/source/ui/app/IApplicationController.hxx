/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
