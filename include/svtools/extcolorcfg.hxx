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
#pragma once

#include <config_options.h>
#include <svtools/svtdllapi.h>
#include <rtl/ustring.hxx>
#include <svl/SfxBroadcaster.hxx>
#include <svl/lstner.hxx>
#include <tools/color.hxx>
#include <memory>
#include <utility>


namespace svtools {

class ExtendedColorConfig_Impl;

class ExtendedColorConfigValue
{
    OUString m_sName;
    OUString m_sDisplayName;
    Color       m_nColor;
    Color       m_nDefaultColor;
public:
    ExtendedColorConfigValue() : m_nColor(0),m_nDefaultColor(0){}
    ExtendedColorConfigValue(OUString  _sName
                            ,OUString  _sDisplayName
                            ,Color      _nColor
                            ,Color      _nDefaultColor)
    : m_sName(std::move(_sName))
    ,m_sDisplayName(std::move(_sDisplayName))
    ,m_nColor(_nColor)
    ,m_nDefaultColor(_nDefaultColor)
    {}

    const OUString& getName()         const { return m_sName; }
    const OUString& getDisplayName()  const { return m_sDisplayName; }
    Color           getColor()        const { return m_nColor; }
    Color           getDefaultColor() const { return m_nDefaultColor; }

    void            setColor(Color _nColor) { m_nColor = _nColor; }
};

class UNLESS_MERGELIBS_MORE(SVT_DLLPUBLIC) ExtendedColorConfig final : public SfxBroadcaster, public SfxListener
{
    friend class ExtendedColorConfig_Impl;
private:
    static ExtendedColorConfig_Impl* m_pImpl;
public:
    ExtendedColorConfig();
    virtual ~ExtendedColorConfig() override;

    virtual void                    Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    // get the configured value
    ExtendedColorConfigValue        GetColorValue(const OUString& _sComponentName,const OUString& _sName)const;
    sal_Int32                       GetComponentCount() const;
    OUString                        GetComponentName(sal_uInt32 _nPos) const;
    OUString                        GetComponentDisplayName(const OUString& _sComponentName) const;
    sal_Int32                       GetComponentColorCount(const OUString& _sName) const;
    ExtendedColorConfigValue        GetComponentColorConfigValue(const OUString& _sComponentName,sal_uInt32 _nPos) const;
};

class SVT_DLLPUBLIC EditableExtendedColorConfig
{
    std::unique_ptr<ExtendedColorConfig_Impl> m_pImpl;
    bool                        m_bModified;
public:
    EditableExtendedColorConfig();
    ~EditableExtendedColorConfig();

    void                        DeleteScheme(const OUString& rScheme );
    void                        AddScheme(const OUString& rScheme );
    void                        LoadScheme(const OUString& rScheme );
    void                        SetCurrentSchemeName(const OUString& rScheme);

    sal_Int32                   GetComponentCount() const;
    OUString                    GetComponentName(sal_uInt32 _nPos) const;
    sal_Int32                   GetComponentColorCount(const OUString& _sName) const;
    ExtendedColorConfigValue    GetComponentColorConfigValue(const OUString& _sName,sal_uInt32 _nPos) const;
    void                        SetColorValue(const OUString& _sComponentName, const ExtendedColorConfigValue& rValue);
    void                        SetModified();
    void                        ClearModified() {m_bModified = false;}
    bool                        IsModified() const {return m_bModified;}
    void                        Commit();

    void                        DisableBroadcast();
    void                        EnableBroadcast();
};
}//namespace svtools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
