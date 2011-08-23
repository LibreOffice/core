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

#ifndef SVX_UNOSHTXT_HXX
#define SVX_UNOSHTXT_HXX

#include <memory>

#ifndef _SVX_UNOEDSRC_HXX
#include <bf_svx/unoedsrc.hxx>
#endif
class Window;
namespace binfilter {

class SvxTextForwarder;
class SdrObject;
class SdrView;
class SvxTextEditSourceImpl;

class SvxTextEditSource : public SvxEditSource, public SvxViewForwarder
{
public:
    SvxTextEditSource( SdrObject* pObj );

    virtual ~SvxTextEditSource();

    virtual SvxEditSource*			Clone() const;
    virtual SvxTextForwarder*		GetTextForwarder();
     virtual SvxViewForwarder*		GetViewForwarder();
     virtual SvxEditViewForwarder*	GetEditViewForwarder( sal_Bool bCreate = sal_False );
    virtual void					UpdateData();

    virtual SfxBroadcaster&			GetBroadcaster() const;

    void lock();
    void unlock();

    static sal_Bool hasLevels( const SdrObject* pObject );

    // the viewforwarder interface
    virtual BOOL		IsValid() const;
    virtual Rectangle	GetVisArea() const;
    virtual Point		LogicToPixel( const Point&, const MapMode& ) const;
    virtual Point		PixelToLogic( const Point&, const MapMode& ) const;

private:
    SvxTextEditSource( SvxTextEditSourceImpl* pImpl );

    SvxTextEditSourceImpl*	mpImpl;
};

}//end of namespace binfilter
#endif
