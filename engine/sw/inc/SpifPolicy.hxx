/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_INC_SPIFPOLICY_HXX
#define INCLUDED_SW_INC_SPIFPOLICY_HXX

#include "swdllapi.h"
#include <rtl/ustring.hxx>
#include <vector>

class SvStream;

namespace sw::seclabel
{
/// A classification level defined by a SPIF policy (xmlspif.org).
struct SpifClassification
{
    OUString aName; ///< securityClassification/@name; default marking phrase
    OUString aColor; ///< @color: W3C name or #RRGGBB; may be empty
    sal_Int32 nLacv = 0; ///< @lacv (label and certificate value)
    sal_Int32 nHierarchy = 0; ///< @hierarchy (dominance ordering)
    bool bObsolete = false; ///< @obsolete: not for newly created labels
};

/// A parsed SPIF policy. Currently models the policy identifier and the
/// classifications; tag sets, privacy marks and marking rules follow.
class SW_DLLPUBLIC SpifPolicy
{
public:
    OUString aName; ///< securityPolicyId/@name
    OUString aId; ///< securityPolicyId/@id (OID)
    std::vector<SpifClassification> aClassifications;

    /// Parse a SPIF document from rStream into this instance. Returns false if
    /// the stream is not a SPIF document.
    bool parse(SvStream& rStream);
};

} // namespace sw::seclabel

#endif // INCLUDED_SW_INC_SPIFPOLICY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
