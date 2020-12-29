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
#ifndef INCLUDED_SVGIO_INC_SVGTITLEDESCNODE_HXX
#define INCLUDED_SVGIO_INC_SVGTITLEDESCNODE_HXX

#include <sal/config.h>

#include <string_view>

#include "svgnode.hxx"


namespace svgio::svgreader
    {
        class SvgTitleDescNode final : public SvgNode
        {
        private:
            /// contained chars
            OUString           maText;

        public:
            SvgTitleDescNode(
                SVGToken aType,
                SvgDocument& rDocument,
                SvgNode* pParent);
            virtual ~SvgTitleDescNode() override;

            /// add new chars
            void concatenate(std::u16string_view rChars);

            /// x content, set if found in current context
            const OUString& getText() const { return maText; }
        };

} // end of namespace svgio::svgreader


#endif // INCLUDED_SVGIO_INC_SVGTITLEDESCNODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
