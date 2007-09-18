/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UndoGuard.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-18 14:58:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef CHART2_UNDOGUARD_HXX
#define CHART2_UNDOGUARD_HXX

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/chart2/XUndoManager.hpp>

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
            ::com::sun::star::chart2::XUndoManager > & xUndoManager
        , const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xModel );
    virtual ~UndoGuard_Base();

    void commitAction();

protected:
    ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > m_xModel;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XUndoManager > m_xUndoManager;

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
            ::com::sun::star::chart2::XUndoManager > & xUndoManager
        , const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xModel );
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
            ::com::sun::star::chart2::XUndoManager > & xUndoManager
        , const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xModel );
    virtual ~UndoLiveUpdateGuard();
};

/** Same as UndoLiveUpdateGuard but with additional storage of the chart's data.
    Only use this if the data has internal data.
 */
class UndoLiveUpdateGuardWithData : public UndoGuard_Base
{
public:
    explicit UndoLiveUpdateGuardWithData( const rtl::OUString& rUndoMessage
        , const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XUndoManager > & xUndoManager
        , const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xModel );
    virtual ~UndoLiveUpdateGuardWithData();
};

class UndoGuardWithSelection : public UndoGuard_Base
{
public:
    explicit UndoGuardWithSelection( const rtl::OUString& rUndoMessage
        , const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XUndoManager > & xUndoManager
        , const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xModel );
    virtual ~UndoGuardWithSelection();
};

}
// CHART2_UNDOGUARD_HXX
#endif
