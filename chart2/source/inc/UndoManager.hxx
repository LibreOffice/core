/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UndoManager.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:23:57 $
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
#ifndef CHART2_UNDOMANAGER_HXX
#define CHART2_UNDOMANAGER_HXX

#include "ConfigItemListener.hxx"

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYBROADCASTER_HPP_
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYLISTENER_HPP_
#include <com/sun/star/util/XModifyListener.hpp>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

// for pair
#include <utility>
// for auto_ptr
#include <memory>

namespace com { namespace sun { namespace star {
namespace frame {
    class XModel;
}
}}}

namespace chart
{
// ----------------------------------------
namespace impl
{

class  UndoStepsConfigItem;
class UndoElement;
class  UndoStack;
class  ModifyBroadcaster;

} // namespace impl
// ----------------------------------------

/** Manages undo by storing the entire XModel in the undo- and redo-buffers.
    Note, that therefore this should not be used for "big" XModels.

    A prerequisite for this to work is that the XModels added to the undo-
    redo-stacks support the css::util::XCloneable interface, which is
    implemented such that the entire model is cloned.
 */
class UndoManager : public ConfigItemListener
{
public:
    explicit UndoManager();
    virtual ~UndoManager();

    /// call this before you change the xCurrentModel
    void preAction( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::frame::XModel > & xCurrentModel );
    /// call this after you successfully did changes to your current model
    void postAction( const ::rtl::OUString & rUndoString );
    /// call this if you aborted the current action.
    void cancelAction();
    /** same as cancelAction() but restores the given model to the state set in
        preAction.  This is useful for cancellation in live-preview dialogs.
    */
    void cancelActionUndo( const ::com::sun::star::uno::Reference<
                               ::com::sun::star::frame::XModel > & xModelToRestore );

    /// same as preAction but with cloning the data for the chart in addition to the model
    void preActionWithData( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::frame::XModel > & xCurrentModel );

    /// give the current model to be put into the redo-stack
    void undo( const ::com::sun::star::uno::Reference<
               ::com::sun::star::frame::XModel > & xCurrentModel );
    /// give the current model to be put into the undo-stack
    void redo( const ::com::sun::star::uno::Reference<
               ::com::sun::star::frame::XModel > & xCurrentModel );

    bool canUndo() const;
    bool canRedo() const;

    ::rtl::OUString getCurrentUndoString() const;
    ::rtl::OUString getCurrentRedoString() const;

    ::com::sun::star::uno::Sequence< ::rtl::OUString > getUndoStrings() const;
    ::com::sun::star::uno::Sequence< ::rtl::OUString > getRedoStrings() const;

    /** limits the size of the undo- and the redo-stack

        The default is defined by the registry entry Common/Undo/Steps.
        (Default is 100, the allowed range is 1 to 1000)
    */
//     void setMaximumNumberOfUndoSteps( sal_Int32 nMaxUndoSteps );
    /// gets the current maximum size of the undo- and the redo-stack
    sal_Int32 getMaximumNumberOfUndoSteps();

    void addModifyListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::util::XModifyListener >& xListener );
    void removeModifyListener( const ::com::sun::star::uno::Reference<
                                   ::com::sun::star::util::XModifyListener >& xListener );

protected:
    // ____ ConfigItemListener ____
    virtual void notify( const ::rtl::OUString & rPropertyName );

private:
    void retrieveConfigUndoSteps();
    void fireModifyEvent();
    void impl_undoRedo(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xCurrentModel,
        impl::UndoStack * pStackToRemoveFrom,
        impl::UndoStack * pStackToAddTo );

    ::std::auto_ptr< impl::UndoStack > m_apUndoStack;
    ::std::auto_ptr< impl::UndoStack > m_apRedoStack;

    impl::UndoElement *  m_pLastRemeberedUndoElement;

    ::std::auto_ptr< impl::UndoStepsConfigItem > m_apUndoStepsConfigItem;
    sal_Int32   m_nMaxNumberOfUndos;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::util::XModifyBroadcaster > m_xModifyBroadcaster;
    // pointer is valid as long as m_xModifyBroadcaster.is()
    impl::ModifyBroadcaster * m_pModifyBroadcaster;
};

} //  namespace chart

// CHART2_UNDOMANAGER_HXX
#endif
