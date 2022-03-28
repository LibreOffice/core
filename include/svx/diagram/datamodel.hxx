/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVX_DIAGRAM_DATAMODEL_HXX
#define INCLUDED_SVX_DIAGRAM_DATAMODEL_HXX

#include <vector>
#include <optional>

#include <svx/svxdllapi.h>
#include <rtl/ustring.hxx>

namespace svx::diagram {

enum TypeConstant {
    XML_none = 0,
    XML_type = 395,
    XML_asst = 680,
    XML_doc = 1924,
    XML_node = 3596,
    XML_norm = 3609,
    XML_parOf = 3878,
    XML_parTrans = 3879,
    XML_pres = 4085,
    XML_presOf = 4090,
    XML_presParOf = 4091,
    XML_rel = 4298,
    XML_sibTrans = 4746,
};

/** A Connection
 */
struct SVXCORE_DLLPUBLIC Connection
{
    Connection();

    TypeConstant mnXMLType;
    OUString msModelId;
    OUString msSourceId;
    OUString msDestId;
    OUString msParTransId;
    OUString msPresId;
    OUString msSibTransId;
    sal_Int32 mnSourceOrder;
    sal_Int32 mnDestOrder;

};

typedef std::vector< Connection > Connections;

/** A point
 */
struct SVXCORE_DLLPUBLIC Point
{
    Point();

    OUString msCnxId;
    OUString msModelId;
    OUString msColorTransformCategoryId;
    OUString msColorTransformTypeId;
    OUString msLayoutCategoryId;
    OUString msLayoutTypeId;
    OUString msPlaceholderText;
    OUString msPresentationAssociationId;
    OUString msPresentationLayoutName;
    OUString msPresentationLayoutStyleLabel;
    OUString msQuickStyleCategoryId;
    OUString msQuickStyleTypeId;

    TypeConstant mnXMLType;
    sal_Int32     mnMaxChildren;
    sal_Int32     mnPreferredChildren;
    sal_Int32     mnDirection;
    std::optional<sal_Int32> moHierarchyBranch;
    sal_Int32     mnResizeHandles;
    sal_Int32     mnCustomAngle;
    sal_Int32     mnPercentageNeighbourWidth;
    sal_Int32     mnPercentageNeighbourHeight;
    sal_Int32     mnPercentageOwnWidth;
    sal_Int32     mnPercentageOwnHeight;
    sal_Int32     mnIncludeAngleScale;
    sal_Int32     mnRadiusScale;
    sal_Int32     mnWidthScale;
    sal_Int32     mnHeightScale;
    sal_Int32     mnWidthOverride;
    sal_Int32     mnHeightOverride;
    sal_Int32     mnLayoutStyleCount;
    sal_Int32     mnLayoutStyleIndex;

    bool          mbOrgChartEnabled;
    bool          mbBulletEnabled;
    bool          mbCoherent3DOffset;
    bool          mbCustomHorizontalFlip;
    bool          mbCustomVerticalFlip;
    bool          mbCustomText;
    bool          mbIsPlaceholder;
};

typedef std::vector< Point >        Points;

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
