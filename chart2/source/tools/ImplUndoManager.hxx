/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ImplUndoManager.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-18 15:08:43 $
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
#ifndef CHART2_IMPLUNDOMANAGER_HXX
#define CHART2_IMPLUNDOMANAGER_HXX

#include "ConfigItemListener.hxx"

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <rtl/ustring.hxx>
#include <unotools/configitem.hxx>

#include <utility>
#include <deque>

namespace com { namespace sun { namespace star {
namespace chart2 {
    class XInternalDataProvider;
}
}}}


namespace chart
{
namespace impl
{

class UndoElement
{
public:
    UndoElement( const ::rtl::OUString & rActionString,
                 const ::com::sun::star::uno::Reference<
                     ::com::sun::star::frame::XModel > & xModel );
    UndoElement( const ::com::sun::star::uno::Reference<
                     ::com::sun::star::frame::XModel > & xModel );
    UndoElement( const UndoElement & rOther );
    virtual ~UndoElement();

    virtual void dispose();
    virtual UndoElement * createFromModel(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xModel );

    virtual void applyToModel(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xInOutModelToChange );

    void setActionString( const ::rtl::OUString & rActionString );
    ::rtl::OUString getActionString() const;

    static ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > cloneModel(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & xModel );

    static void applyModelContentToModel(
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & xInOutModelToChange,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & xModelToCopyFrom,
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XInternalDataProvider > & xData = 0 );

protected:
    ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > m_xModel;

private:
    void initialize( const ::com::sun::star::uno::Reference<
                         ::com::sun::star::frame::XModel > & xModel );

    ::rtl::OUString                           m_aActionString;
};

class UndoElementWithData : public UndoElement
{
public:
    UndoElementWithData( const ::rtl::OUString & rActionString,
                         const ::com::sun::star::uno::Reference<
                             ::com::sun::star::frame::XModel > & xModel );
    UndoElementWithData( const ::com::sun::star::uno::Reference<
                             ::com::sun::star::frame::XModel > & xModel );
    UndoElementWithData( const UndoElementWithData & rOther );
    virtual ~UndoElementWithData();

    virtual void dispose();
    virtual UndoElement * createFromModel(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xModel );

    virtual void applyToModel(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xInOutModelToChange );

private:
    void initializeData();

    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XInternalDataProvider > m_xData;
};

class UndoElementWithSelection : public UndoElement
{
public:
    UndoElementWithSelection( const ::rtl::OUString & rActionString,
                              const ::com::sun::star::uno::Reference<
                                  ::com::sun::star::frame::XModel > & xModel );
    UndoElementWithSelection( const ::com::sun::star::uno::Reference<
                                  ::com::sun::star::frame::XModel > & xModel );
    UndoElementWithSelection( const UndoElementWithSelection & rOther );
    virtual ~UndoElementWithSelection();

    virtual void dispose();
    virtual UndoElement * createFromModel(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xModel );

    virtual void applyToModel(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xInOutModelToChange );

private:
    void initialize( const ::com::sun::star::uno::Reference<
                         ::com::sun::star::frame::XModel > & xModel );

    ::com::sun::star::uno::Any m_aSelection;
};

/** Note that all models that are put into this container are at some point
    disposed of inside this class.  (At least in the destructor).  That means
    the models retrieved here should never be used, but instead their content
    should be copied to a living model.
 */
class UndoStack
{
public:
    UndoStack();
    // disposes of all models left in the stack
    ~UndoStack();

    // removes he last undo action and disposes of the model
    void pop();
    void push( UndoElement * rElement );

    // precondition: !empty()
    UndoElement * top() const;
    ::rtl::OUString topUndoString() const;

    ::com::sun::star::uno::Sequence< ::rtl::OUString > getUndoStrings() const;

    bool empty() const;
    void disposeAndClear();

    // removes all actions that have been inserted more than nMaxSize steps ago.
    // The models of those actions are disposed of
    void limitSize( sal_Int32 nMaxSize );
    sal_Int32 getCurrentLimitation() const;

private:
    void applyLimitation();

    typedef ::std::deque< UndoElement * > tUndoStackType;

    tUndoStackType  m_aStack;
    sal_Int32       m_nSizeLimit;
};

// ----------------------------------------

class UndoStepsConfigItem : public ::utl::ConfigItem
{
public:
    explicit UndoStepsConfigItem( ConfigItemListener & rListener );
    virtual ~UndoStepsConfigItem();

    sal_Int32 getUndoSteps();

protected:
    // ____ ::utl::ConfigItem ____
    virtual void Notify( const ::com::sun::star::uno::Sequence< ::rtl::OUString > & aPropertyNames );

private:
    ConfigItemListener &    m_rListener;
};


} // namespace impl
} //  namespace chart

// CHART2_IMPLUNDOMANAGER_HXX
#endif
