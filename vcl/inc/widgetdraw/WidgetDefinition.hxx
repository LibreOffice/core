/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_WIDGETDEFINITION_HXX
#define INCLUDED_VCL_INC_WIDGETDEFINITION_HXX

#include <vcl/dllapi.h>
#include <memory>
#include <rtl/ustring.hxx>
#include <tools/color.hxx>
#include <unordered_map>
#include <vector>
#include <cstddef>
#include <boost/functional/hash.hpp>
#include <vcl/salnativewidgets.hxx>

namespace vcl
{
enum class WidgetDrawActionType
{
    RECTANGLE,
    LINE,
    IMAGE,
    EXTERNAL
};

class VCL_DLLPUBLIC WidgetDrawAction
{
public:
    WidgetDrawAction(WidgetDrawActionType aType)
        : maType(aType)
    {
    }

    WidgetDrawActionType maType;
};

class VCL_DLLPUBLIC WidgetDrawActionShape : public WidgetDrawAction
{
public:
    WidgetDrawActionShape(WidgetDrawActionType aType)
        : WidgetDrawAction(aType)
        , mnStrokeWidth(-1)
    {
    }

    Color maStrokeColor;
    Color maFillColor;
    sal_Int32 mnStrokeWidth;
};

class VCL_DLLPUBLIC WidgetDrawActionRectangle : public WidgetDrawActionShape
{
public:
    sal_Int32 mnRx;
    sal_Int32 mnRy;

    float mfX1;
    float mfY1;
    float mfX2;
    float mfY2;

    WidgetDrawActionRectangle()
        : WidgetDrawActionShape(WidgetDrawActionType::RECTANGLE)
        , mnRx(0)
        , mnRy(0)
        , mfX1(0.0f)
        , mfY1(0.0f)
        , mfX2(1.0f)
        , mfY2(1.0f)
    {
    }
};

class VCL_DLLPUBLIC WidgetDrawActionLine : public WidgetDrawActionShape
{
public:
    float mfX1;
    float mfY1;
    float mfX2;
    float mfY2;

    WidgetDrawActionLine()
        : WidgetDrawActionShape(WidgetDrawActionType::LINE)
        , mfX1(0.0)
        , mfY1(0.0)
        , mfX2(0.0)
        , mfY2(0.0)
    {
    }
};

class VCL_DLLPUBLIC WidgetDrawActionImage : public WidgetDrawAction
{
public:
    OUString msSource;

    WidgetDrawActionImage()
        : WidgetDrawAction(WidgetDrawActionType::IMAGE)
    {
    }
};

class VCL_DLLPUBLIC WidgetDrawActionExternal : public WidgetDrawAction
{
public:
    OUString msSource;

    WidgetDrawActionExternal()
        : WidgetDrawAction(WidgetDrawActionType::EXTERNAL)
    {
    }
};

struct VCL_DLLPUBLIC ControlTypeAndPart
{
    ControlType meType;
    ControlPart mePart;

    ControlTypeAndPart(ControlType eType, ControlPart ePart)
        : meType(eType)
        , mePart(ePart)
    {
    }

    bool operator==(ControlTypeAndPart const& aOther) const
    {
        return meType == aOther.meType && mePart == aOther.mePart;
    }
};

} // end vcl namespace

namespace std
{
template <> struct VCL_DLLPUBLIC hash<vcl::ControlTypeAndPart>
{
    std::size_t operator()(vcl::ControlTypeAndPart const& rControlTypeAndPart) const noexcept
    {
        std::size_t seed = 0;
        boost::hash_combine(seed, rControlTypeAndPart.meType);
        boost::hash_combine(seed, rControlTypeAndPart.mePart);
        return seed;
    }
};

} // end std namespace

namespace vcl
{
class VCL_DLLPUBLIC WidgetDefinitionState
{
public:
    OString msEnabled;
    OString msFocused;
    OString msPressed;
    OString msRollover;
    OString msDefault;
    OString msSelected;
    OString msButtonValue;
    OString msExtra;

    WidgetDefinitionState(OString const& sEnabled, OString const& sFocused, OString const& sPressed,
                          OString const& sRollover, OString const& sDefault,
                          OString const& sSelected, OString const& sButtonValue,
                          OString const& sExtra);

    std::vector<std::shared_ptr<WidgetDrawAction>> mpWidgetDrawActions;

    void addDrawRectangle(Color aStrokeColor, sal_Int32 nStrokeWidth, Color aFillColor, float fX1,
                          float fY1, float fX2, float fY2, sal_Int32 nRx, sal_Int32 nRy);

    void addDrawLine(Color aStrokeColor, sal_Int32 nStrokeWidth, float fX1, float fY1, float fX2,
                     float fY2);

    void addDrawImage(OUString const& sSource);
    void addDrawExternal(OUString const& sSource);
};

class VCL_DLLPUBLIC WidgetDefinitionPart
{
public:
    sal_Int32 mnWidth;
    sal_Int32 mnHeight;
    sal_Int32 mnMarginWidth;
    sal_Int32 mnMarginHeight;
    OString msOrientation;

    std::vector<std::shared_ptr<WidgetDefinitionState>> getStates(ControlType eType,
                                                                  ControlPart ePart,
                                                                  ControlState eState,
                                                                  ImplControlValue const& rValue);

    std::vector<std::shared_ptr<WidgetDefinitionState>> maStates;
};

class VCL_DLLPUBLIC WidgetDefinitionSettings
{
public:
    OString msCenteredTabs;
};

class VCL_DLLPUBLIC WidgetDefinitionStyle
{
public:
    Color maFaceColor;
    Color maCheckedColor;
    Color maLightColor;
    Color maLightBorderColor;
    Color maShadowColor;
    Color maDarkShadowColor;
    Color maButtonTextColor;
    Color maDefaultActionButtonTextColor;
    Color maActionButtonTextColor;
    Color maActionButtonRolloverTextColor;
    Color maButtonRolloverTextColor;
    Color maRadioCheckTextColor;
    Color maGroupTextColor;
    Color maLabelTextColor;
    Color maWindowColor;
    Color maWindowTextColor;
    Color maDialogColor;
    Color maDialogTextColor;
    Color maWorkspaceColor;
    Color maMonoColor;
    Color maFieldColor;
    Color maFieldTextColor;
    Color maFieldRolloverTextColor;
    Color maActiveColor;
    Color maActiveTextColor;
    Color maActiveBorderColor;
    Color maDeactiveColor;
    Color maDeactiveTextColor;
    Color maDeactiveBorderColor;
    Color maMenuColor;
    Color maMenuBarColor;
    Color maMenuBarRolloverColor;
    Color maMenuBorderColor;
    Color maMenuTextColor;
    Color maMenuBarTextColor;
    Color maMenuBarRolloverTextColor;
    Color maMenuBarHighlightTextColor;
    Color maMenuHighlightColor;
    Color maMenuHighlightTextColor;
    Color maHighlightColor;
    Color maHighlightTextColor;
    Color maActiveTabColor;
    Color maInactiveTabColor;
    Color maTabTextColor;
    Color maTabRolloverTextColor;
    Color maTabHighlightTextColor;
    Color maDisableColor;
    Color maHelpColor;
    Color maHelpTextColor;
    Color maLinkColor;
    Color maVisitedLinkColor;
    Color maToolTextColor;
    Color maFontColor;
};

class VCL_DLLPUBLIC WidgetDefinition
{
public:
    std::shared_ptr<WidgetDefinitionStyle> mpStyle;
    std::shared_ptr<WidgetDefinitionSettings> mpSettings;
    std::unordered_map<ControlTypeAndPart, std::shared_ptr<WidgetDefinitionPart>> maDefinitions;
    std::shared_ptr<WidgetDefinitionPart> getDefinition(ControlType eType, ControlPart ePart);
};

} // end vcl namespace

#endif // INCLUDED_VCL_INC_WIDGETDEFINITION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
