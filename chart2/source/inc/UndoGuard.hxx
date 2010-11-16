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

#include <com/sun/star/document/XUndoManager.hpp>
#include <com/sun/star/frame/XModel.hpp>

// header for class OUString
#include <rtl/ustring.hxx>

#include <boost/shared_ptr.hpp>

namespace chart
{

namespace impl
{
    class ChartModelClone;
}

/** Base Class for UndoGuard and UndoLiveUpdateGuard
*/
class UndoGuard_Base
{
public:
    explicit UndoGuard_Base(
        const ::rtl::OUString& i_undoMessage,
        const ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoManager > & i_undoManager
    );
    ~UndoGuard_Base();

    void    commit();
    void    rollback();

protected:
    void    takeSnapshot( bool i_withData, bool i_withSelection );

    bool    isActionPosted() const { return m_bActionPosted; }

private:
    void    discardSnapshot();

private:
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >           m_xChartModel;
    const ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoManager >  m_xUndoManager;

    ::boost::shared_ptr< impl::ChartModelClone >    m_pDocumentSnapshot;
    rtl::OUString                                   m_aUndoString;
    bool                                            m_bActionPosted;
};

/** A guard which which does nothing, unless you explicitly call commitAction. In particular, in its destructor, it
    does neither auto-commit nor auto-rollback the model changes.
 */
class UndoGuard : public UndoGuard_Base
{
public:
    explicit UndoGuard(
        const ::rtl::OUString& i_undoMessage,
        const ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoManager > & i_undoManager
    );
    ~UndoGuard();
};

/** A guard which, in its destructor, restores the model state it found in the constructor. If
    <member>commitAction</member> is called inbetween, the restouration is not performed.
 */
class UndoLiveUpdateGuard : public UndoGuard_Base
{
public:
    explicit UndoLiveUpdateGuard(
        const ::rtl::OUString& i_undoMessage,
        const ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoManager > & i_undoManager
    );
    ~UndoLiveUpdateGuard();
};

/** Same as UndoLiveUpdateGuard but with additional storage of the chart's data.
    Only use this if the data has internal data.
 */
class UndoLiveUpdateGuardWithData :
        public UndoGuard_Base
{
public:
    explicit UndoLiveUpdateGuardWithData(
        const ::rtl::OUString& i_undoMessage,
        const ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoManager > & i_undoManager
    );
    ~UndoLiveUpdateGuardWithData();
};

class UndoGuardWithSelection : public UndoGuard_Base
{
public:
    explicit UndoGuardWithSelection(
        const ::rtl::OUString& i_undoMessage,
        const ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoManager > & i_undoManager
    );
    virtual ~UndoGuardWithSelection();
};

}
// CHART2_UNDOGUARD_HXX
#endif
