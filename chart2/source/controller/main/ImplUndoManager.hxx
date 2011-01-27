/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef CHART2_IMPLUNDOMANAGER_HXX
#define CHART2_IMPLUNDOMANAGER_HXX

#include "ConfigItemListener.hxx"

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <rtl/ustring.hxx>
#include <unotools/configitem.hxx>

#include <utility>
#include <deque>


class SdrUndoAction;

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

class ShapeUndoElement : public UndoElement
{
public:
    ShapeUndoElement( const ::rtl::OUString& rActionString, SdrUndoAction* pAction );
    ShapeUndoElement( const ShapeUndoElement& rOther );
    virtual ~ShapeUndoElement();

    SdrUndoAction* getSdrUndoAction();

private:
    SdrUndoAction* m_pAction;
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
    virtual void Commit();

private:
    ConfigItemListener &    m_rListener;
};


} // namespace impl
} //  namespace chart

// CHART2_IMPLUNDOMANAGER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
