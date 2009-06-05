/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: accel.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _VCL_ARRANGE_HXX
#define _VCL_ARRANGE_HXX

#include "vcl/window.hxx"

#include <vector>
#include <boost/shared_ptr.hpp>

namespace vcl
{
    /* some helper classes for simple window layouting
       guidelines:
       - a WindowArranger is not a Window
       - a WindowArranger hierarchy manages exactly one level of child windows inside a common parent
         this is to keep the vcl Window hierarchy flat, as some code like accelerators depend on such behavior
       - a WindowArranger never becomes owner of a Window, windows need to be destroyed separately
       - a WindowArranger however always is owner of its child WindowArrangers, that is the
         WindowArranger hierarchy will keep track of its objects and delete them
       - a managed element of a WindowArranger can either be a Window (a leaf in the hierarchy)
         or a child WindowArranger (a node in the hierarchy), but never both
    */

    class WindowArranger
    {
        protected:
        struct Element
        {
            Window*                            m_pElement;
            boost::shared_ptr<WindowArranger>  m_pChild;
            sal_Int32                          m_nExpandPriority;

            Element()
            : m_pElement( NULL )
            , m_pChild()
            , m_nExpandPriority( 0 )
            {}

            Element( Window* i_pWin,
                     boost::shared_ptr<WindowArranger> const & i_pChild = boost::shared_ptr<WindowArranger>(),
                     sal_Int32 i_nExpandPriority = 0
                   )
            : m_pElement( i_pWin )
            , m_pChild( i_pChild )
            , m_nExpandPriority( i_nExpandPriority )
            {}

            void deleteChild() { m_pChild.reset(); }

            sal_Int32 getExpandPriority() const;
        };

        Window*                     m_pParentWindow;
        WindowArranger*             m_pParentArranger;
        Rectangle                   m_aManagedArea;
        long                        m_nOuterBorder;

        public:
        WindowArranger( WindowArranger* i_pParent = NULL )
        : m_pParentWindow( i_pParent ? i_pParent->m_pParentWindow : NULL )
        , m_pParentArranger( i_pParent )
        , m_nOuterBorder( 0 )
        {}
        virtual ~WindowArranger();

        // ask what would be the optimal size
        virtual Size getOptimalSize( WindowSizeType ) const = 0;
        // call Resize to trigger layouting inside the managed area
        // without function while parent window is unset
        virtual void resize() = 0;
        // avoid this if possible, using the constructor instead
        // there can be only one parent window and all managed windows MUST
        // be direct children of that window
        // violating that condition will result in undefined behavior
        virtual void setParentWindow( Window* ) = 0;
        virtual void setParent( WindowArranger* );

        virtual size_t countElements() const = 0;
        virtual boost::shared_ptr<WindowArranger> getChild( size_t i_nIndex ) const = 0;
        virtual Window* getWindow( size_t i_nIndex ) const = 0;
        virtual sal_Int32 getExpandPriority( size_t i_nIndex ) const = 0;

        void setManagedArea( const Rectangle& i_rArea )
        {
            m_aManagedArea = i_rArea;
            resize();
        }

        void setOuterBorder( long i_nBorder )
        {
            m_nOuterBorder = i_nBorder;
            resize();
        }
    };

    class RowOrColumn : public WindowArranger
    {
        long    m_nBorderWidth;
        bool    m_bColumn;

        std::vector< WindowArranger::Element > m_aElements;

        void distributeRowWidth( std::vector< Size >& io_rSizes, long i_nUsedWidth, long i_nExtraWidth );
        public:
        RowOrColumn( WindowArranger* i_pParent = NULL,
                     bool bColumn = true, long i_nBorderWidth = 5 )
        : WindowArranger( i_pParent )
        , m_nBorderWidth( i_nBorderWidth )
        , m_bColumn( bColumn )
        {}

        virtual ~RowOrColumn();

        virtual Size getOptimalSize( WindowSizeType ) const;
        virtual void resize();
        virtual void setParentWindow( Window* );
        virtual size_t countElements() const { return m_aElements.size(); }
        virtual boost::shared_ptr<WindowArranger> getChild( size_t i_nIndex ) const;
        virtual Window* getWindow( size_t i_nIndex ) const;
        virtual sal_Int32 getExpandPriority( size_t i_nIndex ) const;

        // add a managed window at the given index
        // an index smaller than zero means add the window at the end
        void addWindow( Window*, sal_Int32 i_nExpandPrio = 0, sal_Int32 i_nIndex = -1 );
        void remove( Window* );

        void addChild( boost::shared_ptr<WindowArranger> const &, sal_Int32 i_nExpandPrio = 0, sal_Int32 i_nIndex = -1 );
        // convenience: use for addChild( new WindowArranger( ... ) ) constructs
        void addChild( WindowArranger* i_pNewChild, sal_Int32 i_nExpandPrio = 0, sal_Int32 i_nIndex = -1 )
        { addChild( boost::shared_ptr<WindowArranger>( i_pNewChild ), i_nExpandPrio, i_nIndex ); }
        void remove( boost::shared_ptr<WindowArranger> const & );
    };

    class Indenter : public WindowArranger
    {
        long                        m_nIndent;
        WindowArranger::Element     m_aElement;

        public:
        Indenter( WindowArranger* i_pParent = NULL, long i_nIndent = 15 )
        : WindowArranger( i_pParent )
        , m_nIndent( i_nIndent )
        {}

        virtual ~Indenter();

        virtual Size getOptimalSize( WindowSizeType ) const;
        virtual void resize();
        virtual void setParentWindow( Window* );
        virtual size_t countElements() const { return (m_aElement.m_pElement != 0 || m_aElement.m_pChild != 0) ? 1 : 0; }
        virtual boost::shared_ptr<WindowArranger> getChild( size_t i_nIndex ) const { return (i_nIndex == 0) ? m_aElement.m_pChild : boost::shared_ptr<WindowArranger>(); }
        virtual Window* getWindow( size_t i_nIndex ) const { return (i_nIndex == 0) ? m_aElement.m_pElement : NULL; }
        virtual sal_Int32 getExpandPriority( size_t i_nIndex ) const
        { return (i_nIndex == 0) ? m_aElement.getExpandPriority() : 0; }

        void setIndent( long i_nIndent )
        {
            m_nIndent = i_nIndent;
            resize();
        }

        void setWindow( Window*, sal_Int32 i_nExpandPrio = 0 );
        void setChild( boost::shared_ptr<WindowArranger> const &, sal_Int32 i_nExpandPrio = 0 );
        // convenience: use for setChild( new WindowArranger( ... ) ) constructs
        void setChild( WindowArranger* i_pChild, sal_Int32 i_nExpandPrio = 0 )
        { setChild( boost::shared_ptr<WindowArranger>( i_pChild ), i_nExpandPrio ); }
    };

    class Spacer : public WindowArranger
    {
        WindowArranger::Element     m_aElement;
        public:
        Spacer( WindowArranger* i_pParent = NULL, sal_Int32 i_nPrio = 20 )
        : WindowArranger( i_pParent )
        , m_aElement( NULL, boost::shared_ptr<WindowArranger>(), i_nPrio )
        {}

        virtual ~Spacer() {}

        virtual Size getOptimalSize( WindowSizeType ) const
        { return Size( 0, 0 ); }
        virtual void resize() {}
        virtual void setParentWindow( Window* ) {}
        virtual size_t countElements() const { return 1; }
        virtual boost::shared_ptr<WindowArranger> getChild( size_t i_nIndex ) const { return boost::shared_ptr<WindowArranger>(); }
        virtual Window* getWindow( size_t i_nIndex ) const { return NULL; }
        virtual sal_Int32 getExpandPriority( size_t i_nIndex ) const
        { return (i_nIndex == 0) ? m_aElement.getExpandPriority() : 0; }
    };
}

#endif

