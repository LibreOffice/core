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
#ifndef CHART2_UNDOGUARD_HXX
#define CHART2_UNDOGUARD_HXX

#include <com/sun/star/chart2/XDocumentActions.hpp>

// header for class OUString
#include <rtl/ustring.hxx>

namespace chart
{
/** Base Class for UndoGuard and UndoLiveUpdateGuard
*/
class UndoGuard_Base
{
public:
    explicit UndoGuard_Base( const rtl::OUString & rUndoMessage
        , const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDocumentActions > & xDocumentActions );
    virtual ~UndoGuard_Base();

    void commitAction();

protected:
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDocumentActions > m_xDocumentActions;

    rtl::OUString   m_aUndoString;
    bool            m_bActionPosted;
};

/** This guard calls preAction at the given Model in the CTOR and
    cancelAction in the DTOR if no other method is called.
    If commitAction is called the destructor does nothin anymore.
 */
class UndoGuard : public UndoGuard_Base
{
public:
    explicit UndoGuard( const rtl::OUString& rUndoMessage
        , const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDocumentActions > & xDocumentActions );
    virtual ~UndoGuard();
};

/** This guard calls preAction at the given Model in the CTOR and
    cancelActionUndo in the DTOR if no other method is called.
    If commitAction is called the destructor does nothin anymore.
 */
class UndoLiveUpdateGuard : public UndoGuard_Base
{
public:
    explicit UndoLiveUpdateGuard( const rtl::OUString& rUndoMessage
        , const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDocumentActions > & xDocumentActions );
    virtual ~UndoLiveUpdateGuard();
};

/** Same as UndoLiveUpdateGuard but with additional storage of the chart's data.
    Only use this if the data has internal data.
 */
class UndoLiveUpdateGuardWithData :
        public UndoGuard_Base
{
public:
    explicit UndoLiveUpdateGuardWithData( const rtl::OUString& rUndoMessage
        , const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDocumentActions > & xDocumentActions );
    virtual ~UndoLiveUpdateGuardWithData();
};

class UndoGuardWithSelection : public UndoGuard_Base
{
public:
    explicit UndoGuardWithSelection( const rtl::OUString& rUndoMessage
        , const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDocumentActions > & xDocumentActions );
    virtual ~UndoGuardWithSelection();
};

}
// CHART2_UNDOGUARD_HXX
#endif
