/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Miklos Vajna <vmiklos@frugalware.org>
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef _RTFREFERENCEPROPERTIES_HXX_
#define _RTFREFERENCEPROPERTIES_HXX_

#include <map>
#include <resourcemodel/WW8ResourceModel.hxx>
#include <rtfsprm.hxx>

class SvStream;

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
                virtual void resolve(Properties & rHandler);
                virtual std::string getType() const;
            private:
                RTFSprms m_rAttributes;
                RTFSprms m_rSprms;
        };
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFREFERENCEPROPERTIES_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
