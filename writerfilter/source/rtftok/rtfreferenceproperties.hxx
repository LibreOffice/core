/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "rtfsprm.hxx"

namespace writerfilter::rtftok
{
/// Sends RTFSprm instances to DomainMapper.
class RTFReferenceProperties : public writerfilter::Reference<Properties>
{
public:
    RTFReferenceProperties(RTFSprms aAttributes, RTFSprms aSprms);
    explicit RTFReferenceProperties(RTFSprms aAttributes);
    ~RTFReferenceProperties() override;
    void resolve(Properties& rHandler) override;
    RTFSprms& getAttributes() { return m_aAttributes; }
    RTFSprms& getSprms() { return m_aSprms; }

private:
    RTFSprms m_aAttributes;
    RTFSprms m_aSprms;
};
} // namespace writerfilter::rtftok

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
