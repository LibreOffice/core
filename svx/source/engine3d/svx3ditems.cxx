/*************************************************************************
 *
 *  $RCSfile: svx3ditems.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2000-11-24 16:22:01 $
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

#ifndef _SVX3DITEMS_HXX
#include "svx3ditems.hxx"
#endif

#ifndef _COM_SUN_STAR_DRAWING_NORMALSKIND_HPP_
#include <com/sun/star/drawing/NormalsKind.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_TEXTUREPROJECTIONMODE_HPP_
#include <com/sun/star/drawing/TextureProjectionMode.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_TEXTUREKIND_HPP_
#include <com/sun/star/drawing/TextureKind.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_TEXTUREMODE_HPP_
#include <com/sun/star/drawing/TextureMode.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_PROJECTIONMODE_HPP_
#include <com/sun/star/drawing/ProjectionMode.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_SHADEMODE_HPP_
#include <com/sun/star/drawing/ShadeMode.hpp>
#endif


//////////////////////////////////////////////////////////////////////////////

using namespace ::rtl;
using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////

// Svx3DNormalsKindItem: use drawing::NormalsKind
sal_Bool Svx3DNormalsKindItem::QueryValue( uno::Any& rVal, BYTE nMemberId) const
{
    rVal <<= (drawing::NormalsKind)GetValue();
    return sal_True;
}

sal_Bool Svx3DNormalsKindItem::PutValue( const uno::Any& rVal, BYTE nMemberId)
{
    drawing::NormalsKind eVar;
    if(!(rVal >>= eVar))
        return sal_False;
    SetValue((sal_Int16)eVar);
    return sal_True;
}

// Svx3DTextureProjectionXItem: use drawing::TextureProjectionMode
sal_Bool Svx3DTextureProjectionXItem::QueryValue( uno::Any& rVal, BYTE nMemberId) const
{
    rVal <<= (drawing::TextureProjectionMode)GetValue();
    return sal_True;
}

sal_Bool Svx3DTextureProjectionXItem::PutValue( const uno::Any& rVal, BYTE nMemberId)
{
    drawing::TextureProjectionMode eVar;
    if(!(rVal >>= eVar))
        return sal_False;
    SetValue((sal_Int16)eVar);
    return sal_True;
}

// Svx3DTextureProjectionYItem: use drawing::TextureProjectionMode
sal_Bool Svx3DTextureProjectionYItem::QueryValue( uno::Any& rVal, BYTE nMemberId) const
{
    rVal <<= (drawing::TextureProjectionMode)GetValue();
    return sal_True;
}

sal_Bool Svx3DTextureProjectionYItem::PutValue( const uno::Any& rVal, BYTE nMemberId)
{
    drawing::TextureProjectionMode eVar;
    if(!(rVal >>= eVar))
        return sal_False;
    SetValue((sal_Int16)eVar);
    return sal_True;
}

// Svx3DTextureKindItem: use drawing::TextureKind
sal_Bool Svx3DTextureKindItem::QueryValue( uno::Any& rVal, BYTE nMemberId) const
{
    rVal <<= (drawing::TextureKind)GetValue();
    return sal_True;
}

sal_Bool Svx3DTextureKindItem::PutValue( const uno::Any& rVal, BYTE nMemberId)
{
    drawing::TextureKind eVar;
    if(!(rVal >>= eVar))
        return sal_False;
    SetValue((sal_Int16)eVar);
    return sal_True;
}

// Svx3DTextureModeItem: use drawing:TextureMode
sal_Bool Svx3DTextureModeItem::QueryValue( uno::Any& rVal, BYTE nMemberId) const
{
    rVal <<= (drawing::TextureMode)GetValue();
    return sal_True;
}

sal_Bool Svx3DTextureModeItem::PutValue( const uno::Any& rVal, BYTE nMemberId)
{
    drawing::TextureMode eVar;
    if(!(rVal >>= eVar))
        return sal_False;
    SetValue((sal_Int16)eVar);
    return sal_True;
}

// Svx3DPerspectiveItem: use drawing::ProjectionMode
sal_Bool Svx3DPerspectiveItem::QueryValue( uno::Any& rVal, BYTE nMemberId) const
{
    rVal <<= (drawing::ProjectionMode)GetValue();
    return sal_True;
}

sal_Bool Svx3DPerspectiveItem::PutValue( const uno::Any& rVal, BYTE nMemberId)
{
    drawing::ProjectionMode eVar;
    if(!(rVal >>= eVar))
        return sal_False;
    SetValue((sal_Int16)eVar);
    return sal_True;
}

// Svx3DShadeModeItem: use drawing::ShadeMode
sal_Bool Svx3DShadeModeItem::QueryValue( uno::Any& rVal, BYTE nMemberId) const
{
    rVal <<= (drawing::ShadeMode)GetValue();
    return sal_True;
}

sal_Bool Svx3DShadeModeItem::PutValue( const uno::Any& rVal, BYTE nMemberId)
{
    drawing::ShadeMode eVar;
    if(!(rVal >>= eVar))
        return sal_False;
    SetValue((sal_Int16)eVar);
    return sal_True;
}

// EOF
