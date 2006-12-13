/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: waitsymbol.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 15:23:38 $
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

#if ! defined(WAITSYMBOL_HXX_INCLUDED)
#define WAITSYMBOL_HXX_INCLUDED

#include <com/sun/star/rendering/XBitmap.hpp>
#include <cppcanvas/customsprite.hxx>

#include "disposable.hxx"
#include "vieweventhandler.hxx"
#include "eventmultiplexer.hxx"
#include "unoview.hxx"

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/utility.hpp> // for noncopyable
#include <vector>

namespace slideshow {
namespace internal {

typedef boost::shared_ptr<class WaitSymbol> WaitSymbolSharedPtr;

class WaitSymbol : public Disposable,
                   public ViewEventHandler,
                   private ::boost::noncopyable
{
public:
    static WaitSymbolSharedPtr create( const ::com::sun::star::uno::Reference<
                                             ::com::sun::star::rendering::XBitmap>& xBitmap,
                                       EventMultiplexer&                            rEventMultiplexer,
                                       const UnoViewContainer&                      rViewContainer );

    /** Shows the wait symbol.
     */
    void show() { setVisible(true); }

    /** Hides the wait symbol.
     */
    void hide() { setVisible(false); }

    // Disposable:
    virtual void dispose();

    // ViewEventHandler
    void viewAdded( const UnoViewSharedPtr& rView );
    void viewRemoved( const UnoViewSharedPtr& rView );
    void viewChanged( const UnoViewSharedPtr& rView );

private:
    WaitSymbol( const ::com::sun::star::uno::Reference<
                      ::com::sun::star::rendering::XBitmap>& xBitmap,
                EventMultiplexer&                            rEventMultiplexer,
                const UnoViewContainer&                      rViewContainer );

    ::com::sun::star::uno::Reference<
        ::com::sun::star::rendering::XBitmap> m_xBitmap;

    ::basegfx::B2DPoint calcSpritePos( UnoViewSharedPtr const & rView ) const;

    typedef ::std::vector<
        ::std::pair<UnoViewSharedPtr,
                    cppcanvas::CustomSpriteSharedPtr> > ViewsVecT;
    ViewsVecT m_views;

    EventMultiplexer& mrEventMultiplexer;

    template <typename func_type>
    void for_each_sprite( func_type const & func ) const
    {
        ViewsVecT::const_iterator iPos( m_views.begin() );
        const ViewsVecT::const_iterator iEnd( m_views.end() );
        for ( ; iPos != iEnd; ++iPos )
            func( iPos->second );
    }

    bool m_bVisible;
    void setVisible( const bool bVisible );
};

} // namespace internal
} // namespace presentation

#endif
