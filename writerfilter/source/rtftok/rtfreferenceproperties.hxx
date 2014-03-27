/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _RTFREFERENCEPROPERTIES_HXX_
#define _RTFREFERENCEPROPERTIES_HXX_

#include <rtfsprm.hxx>

namespace writerfilter {
    namespace rtftok {
        /// Sends RTFSprm instances to DomainMapper.
        class RTFReferenceProperties
            : public writerfilter::Reference<Properties>
        {
            public:
                RTFReferenceProperties(RTFSprms rAttributes, RTFSprms rSprms);
                RTFReferenceProperties(RTFSprms rAttributes);
                virtual ~RTFReferenceProperties();
                virtual void resolve(Properties & rHandler) SAL_OVERRIDE;
                virtual std::string getType() const SAL_OVERRIDE;
                RTFSprms& getAttributes();
                RTFSprms& getSprms();
            private:
                RTFSprms m_aAttributes;
                RTFSprms m_aSprms;
        };
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFREFERENCEPROPERTIES_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
