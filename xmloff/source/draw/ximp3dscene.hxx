/*************************************************************************
 *
 *  $RCSfile: ximp3dscene.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2000-11-24 16:36:47 $
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

#ifndef _XIMP3DSCENE_HXX
#define _XIMP3DSCENE_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif

#ifndef _SDXMLIMP_HXX
#include "sdxmlimp.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif

#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_HOMOGENMATRIX_HPP_
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_PROJECTIONMODE_HPP_
#include <com/sun/star/drawing/ProjectionMode.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_SHADEMODE_HPP_
#include <com/sun/star/drawing/ShadeMode.hpp>
#endif

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

#ifndef _SVX_VECTOR3D_HXX
#include <goodies/vector3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// dr3d:3dlight context

class SdXML3DLightContext: public SvXMLImportContext
{
    // local parameters which need to be read
    Color                       maDiffuseColor;
    Vector3D                    maDirection;
    BOOL                        mbEnabled;
    BOOL                        mbSpecular;

public:
    SdXML3DLightContext(
        SdXMLImport& rImport,
        sal_uInt16 nPrfx,
        const rtl::OUString& rLName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList);
    virtual ~SdXML3DLightContext();

    const Color& GetDiffuseColor() { return maDiffuseColor; }
    const Vector3D& GetDirection() { return maDirection; }
    BOOL GetEnabled() { return mbEnabled; }
    BOOL GetSpecular() { return mbSpecular; }
};
DECLARE_LIST(Imp3DLightList, SdXML3DLightContext*);

//////////////////////////////////////////////////////////////////////////////
// dr3d:3dscene context

class SdXML3DSceneShapeContext : public SvXMLImportContext
{
    // the shape group this group is working on
    // this is the scene at the same time
    com::sun::star::uno::Reference< com::sun::star::drawing::XShapes > mxShapes;

    // list for local light contexts
    Imp3DLightList              maList;

    // local parameters which need to be read
    sal_Int32                   mnX;
    sal_Int32                   mnY;
    sal_Int32                   mnWidth;
    sal_Int32                   mnHeight;

    com::sun::star::drawing::HomogenMatrix mxHomMat;
    BOOL                        mbSetTransform;

    com::sun::star::drawing::ProjectionMode mxPrjMode;
    sal_Int32                   mnDistance;
    sal_Int32                   mnFocalLength;
    sal_Int32                   mnShadowSlant;
    com::sun::star::drawing::ShadeMode mxShadeMode;
    Color                       maAmbientColor;
    BOOL                        mbLightingMode;

    const SdXMLImport& GetSdImport() const { return (const SdXMLImport&)GetImport(); }
    SdXMLImport& GetSdImport() { return (SdXMLImport&)GetImport(); }

protected:
    void SetLocalShapesContext(com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rNew)
        { mxShapes = rNew; }

public:
    TYPEINFO();

    SdXML3DSceneShapeContext(
        SvXMLImport& rImport,
        USHORT nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);
    virtual ~SdXML3DSceneShapeContext();

    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);
    virtual void EndElement();

    virtual SvXMLImportContext *CreateChildContext(
        USHORT nPrefix, const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList );

    const com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& GetLocalShapesContext() const
        { return mxShapes; }
    com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& GetLocalShapesContext()
        { return mxShapes; }
};


#endif  //  _XIMPGROUP_HXX
