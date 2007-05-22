/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UndoGuard.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:23:47 $
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

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
// header for class OUString
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#include "UndoManager.hxx"

namespace chart
{
/** Base Class for UndoGuard and UndoLiveUpdateGuard
*/
class UndoGuard_Base
{
public:
    explicit UndoGuard_Base( const rtl::OUString& rUndoMessage
        , UndoManager& rUndoManager
        , const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xModel );
    virtual ~UndoGuard_Base();

    void commitAction();

protected:
    ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > m_xModel;

    UndoManager&    m_rUndoManager;
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
        , UndoManager& rUndoManager
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
        , UndoManager& rUndoManager
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
        , UndoManager& rUndoManager
        , const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xModel );
    virtual ~UndoLiveUpdateGuardWithData();
};

}
// CHART2_UNDOGUARD_HXX
#endif
