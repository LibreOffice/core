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

#ifndef OOX_DRAWINGML_TABLECELL_HXX
#define OOX_DRAWINGML_TABLECELL_HXX

#include "oox/helper/propertymap.hxx"
#include "oox/drawingml/color.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/lineproperties.hxx"
#include "oox/drawingml/fillproperties.hxx"
#include "oox/drawingml/textliststyle.hxx"
#include <com/sun/star/table/XCell.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <vector>
#include <map>

namespace oox { namespace drawingml { namespace table {

class TableCellContext;
class TableProperties;
class TableStyle;

class TableCell
{
    friend class TableCellContext;

public:

    TableCell();
    ~TableCell();

    sal_Int32	getRowSpan() const { return mnRowSpan; };
    void		setRowSpan( sal_Int32 nRowSpan ){ mnRowSpan = nRowSpan; };
    sal_Int32	getGridSpan() const { return mnGridSpan; };
    void		setGridSpan( sal_Int32 nGridSpan ){ mnGridSpan = nGridSpan; };
    sal_Bool	gethMerge() const { return mbhMerge; };
    void		sethMerge( sal_Bool bhMerge ){ mbhMerge = bhMerge; };
    sal_Bool	getvMerge() const { return mbvMerge; };
    void		setvMerge( sal_Bool bvMerge ){ mbvMerge = bvMerge; };
    sal_Int32	getLeftMargin() const { return mnMarL; };
    void		setLeftMargin( sal_Int32 nMargin ){ mnMarL = nMargin; };
    sal_Int32	getRightMargin() const { return mnMarR; };
    void		setRightMargin( sal_Int32 nMargin ){ mnMarR = nMargin; };
    sal_Int32	getTopMargin() const { return mnMarT; };
    void		setTopMargin( sal_Int32 nMargin ){ mnMarT = nMargin; };
    sal_Int32	getBottomMargin() const { return mnMarB; };
    void		setBottomMargin( sal_Int32 nMargin ){ mnMarB = nMargin; };
    sal_Int32	getVertToken() const { return mnVertToken; };
    void		setVertToken( sal_Int32 nToken ){ mnVertToken = nToken; };
    sal_Int32	getAnchorToken() const { return mnAnchorToken; };
    void		setAnchorToken( sal_Int32 nToken ){ mnAnchorToken = nToken; };
    sal_Bool	getAnchorCtr() const { return mbAnchorCtr; };
    void		setAnchorCtr( sal_Bool bAnchorCtr ){ mbAnchorCtr = bAnchorCtr; };
    sal_Int32	getHorzOverflowToken() const { return mnHorzOverflowToken; };
    void		setHorzOverflowToken( sal_Int32 nToken ){ mnHorzOverflowToken = nToken; };

    void						setTextBody( const oox::drawingml::TextBodyPtr& pTextBody ){ mpTextBody = pTextBody; };
    oox::drawingml::TextBodyPtr	getTextBody(){ return mpTextBody; };

    void pushToXCell( const ::oox::core::XmlFilterBase& rFilterBase, ::oox::drawingml::TextListStylePtr pMasterTextListStyle,
        const ::com::sun::star::uno::Reference < ::com::sun::star::table::XCell >& rxCell, const TableProperties& rTableProperties,
            const TableStyle& rTable, sal_Int32 nColumn, sal_Int32 nMaxColumn, sal_Int32 nRow, sal_Int32 nMaxRow );

private:

    oox::drawingml::TextBodyPtr mpTextBody;

    oox::drawingml::LineProperties	maLinePropertiesLeft;
    oox::drawingml::LineProperties	maLinePropertiesRight;
    oox::drawingml::LineProperties	maLinePropertiesTop;
    oox::drawingml::LineProperties	maLinePropertiesBottom;
    oox::drawingml::LineProperties	maLinePropertiesTopLeftToBottomRight;
    oox::drawingml::LineProperties	maLinePropertiesBottomLeftToTopRight;

    oox::drawingml::FillProperties	maFillProperties;

    sal_Int32 mnRowSpan;
    sal_Int32 mnGridSpan;
    sal_Bool mbhMerge;
    sal_Bool mbvMerge;

    sal_Int32 mnMarL;
    sal_Int32 mnMarR;
    sal_Int32 mnMarT;
    sal_Int32 mnMarB;
    sal_Int32 mnVertToken;
    sal_Int32 mnAnchorToken;
    sal_Bool  mbAnchorCtr;
    sal_Int32 mnHorzOverflowToken;
};

} } }

#endif  //  OOX_DRAWINGML_TABLECELL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
