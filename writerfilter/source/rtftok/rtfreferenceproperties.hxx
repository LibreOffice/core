/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFREFERENCEPROPERTIES_HXX
#define INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFREFERENCEPROPERTIES_HXX

#include "rtfsprm.hxx"

namespace writerfilter
{
namespace rtftok
{
/// Sends RTFSprm instances to DomainMapper.
class RTFReferenceProperties : public writerfilter::Reference<Properties>
{
public:
    RTFReferenceProperties(const RTFSprms& rAttributes, const RTFSprms& rSprms);
    explicit RTFReferenceProperties(const RTFSprms& rAttributes);
    ~RTFReferenceProperties() override;
    void resolve(Properties& rHandler) override;
    RTFSprms& getAttributes() { return m_aAttributes; }
    RTFSprms& getSprms() { return m_aSprms; }

private:
    RTFSprms m_aAttributes;
    RTFSprms m_aSprms;
};
} // namespace rtftok
} // namespace writerfilter

#endif // INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFREFERENCEPROPERTIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
