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

#ifndef SVTOOLS_MOUSEFUNCTION_HXX
#define SVTOOLS_MOUSEFUNCTION_HXX

#include "svtools/table/tabletypes.hxx"

#include <rtl/ref.hxx>

#include <boost/noncopyable.hpp>

class MouseEvent;

//......................................................................................................................
namespace svt { namespace table
{
//......................................................................................................................

    class ITableControl;

    //==================================================================================================================
    //= FunctionResult
    //==================================================================================================================
    enum FunctionResult
    {
        ActivateFunction,
        ContinueFunction,
        DeactivateFunction,

        SkipFunction
    };

    //==================================================================================================================
    //= IMouseFunction
    //==================================================================================================================
    class IMouseFunction : public ::rtl::IReference, public ::boost::noncopyable
    {
    public:
        virtual FunctionResult  handleMouseMove( ITableControl& i_tableControl, MouseEvent const & i_event ) = 0;
        virtual FunctionResult  handleMouseDown( ITableControl& i_tableControl, MouseEvent const & i_event ) = 0;
        virtual FunctionResult  handleMouseUp( ITableControl& i_tableControl, MouseEvent const & i_event ) = 0;

    protected:
        virtual ~IMouseFunction() { }
    };

    //==================================================================================================================
    //= MouseFunction
    //==================================================================================================================
    class MouseFunction : public IMouseFunction
    {
    public:
        MouseFunction()
            :m_refCount( 0 )
        {
        }
    protected:
        ~MouseFunction()
        {
        }

    public:
        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();

    private:
        oslInterlockedCount m_refCount;
    };

    //==================================================================================================================
    //= ColumnResize
    //==================================================================================================================
    class ColumnResize : public MouseFunction
    {
    public:
        ColumnResize()
            :m_nResizingColumn( COL_INVALID )
        {
        }

    public:
        // IMouseFunction
        virtual FunctionResult  handleMouseMove( ITableControl& i_tableControl, MouseEvent const & i_event );
        virtual FunctionResult  handleMouseDown( ITableControl& i_tableControl, MouseEvent const & i_event );
        virtual FunctionResult  handleMouseUp( ITableControl& i_tableControl, MouseEvent const & i_event );

    private:
        ColPos  m_nResizingColumn;
    };

    //==================================================================================================================
    //= RowSelection
    //==================================================================================================================
    class RowSelection : public MouseFunction
    {
    public:
        RowSelection()
            :m_bActive( false )
        {
        }

    public:
        // IMouseFunction
        virtual FunctionResult  handleMouseMove( ITableControl& i_tableControl, MouseEvent const & i_event );
        virtual FunctionResult  handleMouseDown( ITableControl& i_tableControl, MouseEvent const & i_event );
        virtual FunctionResult  handleMouseUp( ITableControl& i_tableControl, MouseEvent const & i_event );

    private:
        bool    m_bActive;
    };

    //==================================================================================================================
    //= ColumnSortHandler
    //==================================================================================================================
    class ColumnSortHandler : public MouseFunction
    {
    public:
        ColumnSortHandler()
            :m_nActiveColumn( COL_INVALID )
        {
        }

    public:
        // IMouseFunction
        virtual FunctionResult  handleMouseMove( ITableControl& i_tableControl, MouseEvent const & i_event );
        virtual FunctionResult  handleMouseDown( ITableControl& i_tableControl, MouseEvent const & i_event );
        virtual FunctionResult  handleMouseUp( ITableControl& i_tableControl, MouseEvent const & i_event );

    private:
        ColPos  m_nActiveColumn;
    };

//......................................................................................................................
} } // namespace svt::table
//......................................................................................................................

#endif // SVTOOLS_MOUSEFUNCTION_HXX
