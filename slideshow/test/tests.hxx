/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tests.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-17 15:21:41 $
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

#ifndef INCLUDED_TESTS_HXX
#define INCLUDED_TESTS_HXX

#include "animatableshape.hxx"
#include "unoview.hxx"
#include <boost/shared_ptr.hpp>

namespace basegfx{ class B1DRange; class B2DRange; class B2DVector; }

class TestView : public slideshow::internal::UnoView
{
public:
    /// true iff clear() has been called
    virtual bool isClearCalled() const = 0;
    virtual std::vector<std::pair<basegfx::B2DVector,double> > getCreatedSprites() const = 0;
    virtual basegfx::B1DRange getPriority() const = 0;
    /// true iff setClip was called (on and off)
    virtual bool wasClipSet() const = 0;
    virtual basegfx::B2DRange getBounds() const = 0;

    virtual std::vector<boost::shared_ptr<TestView> > getViewLayers() const = 0;
};

typedef boost::shared_ptr<TestView> TestViewSharedPtr;
TestViewSharedPtr createTestView();


///////////////////////////////////////////////////////////////////////////////////////


class TestShape : public slideshow::internal::AnimatableShape
{
public:
    virtual std::vector<
    std::pair<slideshow::internal::ViewLayerSharedPtr,bool> > getViewLayers() const = 0;
    virtual sal_Int32 getNumUpdates() const = 0;
    virtual sal_Int32 getNumRenders() const = 0;
    virtual sal_Int32 getAnimationCount() const = 0;
};

typedef boost::shared_ptr<TestShape> TestShapeSharedPtr;
TestShapeSharedPtr createTestShape(const basegfx::B2DRange& rRect,
                                   double                   nPrio);

#endif /* INCLUDED_TESTS_HXX */
