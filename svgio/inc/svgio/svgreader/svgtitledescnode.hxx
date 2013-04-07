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
#ifndef INCLUDED_SVGIO_SVGREADER_SVGTITLEDESCNODE_HXX
#define INCLUDED_SVGIO_SVGREADER_SVGTITLEDESCNODE_HXX

#include <svgio/svgreader/svgnode.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        class SvgTitleDescNode : public SvgNode
        {
        private:
            /// contained chars
            OUString           maText;

        public:
            SvgTitleDescNode(
                SVGToken aType,
                SvgDocument& rDocument,
                SvgNode* pParent);
            virtual ~SvgTitleDescNode();

            /// add new chars
            void concatenate(const OUString& rChars);

            /// x content, set if found in current context
            const OUString& getText() const { return maText; }
        };
    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SVGIO_SVGREADER_SVGTITLEDESCNODE_HXX

// eof
