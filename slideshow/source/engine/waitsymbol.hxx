/*************************************************************************
 *
 *  $RCSfile: waitsymbol.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-03-30 08:01:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#if ! defined(WAITSYMBOL_HXX_INCLUDED)
#define WAITSYMBOL_HXX_INCLUDED

#include <com/sun/star/rendering/XBitmap.hpp>

#include "cppcanvas/customsprite.hxx"
#include "disposable.hxx"
#include "eventmultiplexer.hxx"
#include "unoview.hxx"

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/utility.hpp> // for noncopyable

#include <vector>


namespace presentation {
namespace internal {

class WaitSymbol : public Disposable, private boost::noncopyable
{
public:
    WaitSymbol( const com::sun::star::uno::Reference<
                    com::sun::star::rendering::XBitmap>&    xBitmap,
                EventMultiplexer&                           rEventMultiplexer );

    /** Shows the wait symbol.
     */
    void show() { setVisible(true); }

    /** Hides the wait symbol.
     */
    void hide() { setVisible(false); }

    /** Adds a view for display.
     */
    void addView( UnoViewSharedPtr const & rView );

    void removeView( UnoViewSharedPtr const & rView );

    void notifyViewChange();

    // Disposable:
    virtual void dispose();

private:
    com::sun::star::uno::Reference<
        com::sun::star::rendering::XBitmap> m_xBitmap;

    basegfx::B2DPoint calcSpritePos( UnoViewSharedPtr const & rView ) const;

    typedef std::vector<
        std::pair<UnoViewSharedPtr,
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
