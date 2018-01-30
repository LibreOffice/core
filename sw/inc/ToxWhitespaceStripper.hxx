/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#ifndef TOXWHITESPACESTRIPPER_HXX_
#define TOXWHITESPACESTRIPPER_HXX_

#include <rtl/ustring.hxx>
#include <vector>

namespace sw {

/** This class helps to remove unwanted whitespaces from a string to use in a Tox.
 *
 * The new string will have
 * - Newlines changed to spaces
 * - Consecutive spaces merged
 * - Trailing spaces removed
 *
 * It also allows to find the corresponding new positions of the input string in the stripped string.
 * This is important for attributes which might have to be imported, e.g., it helps to answer the question:
 * The 3rd character of the input string is subscript, which character in the output string is that?
 *
 * @note One leading whitespace is preserved.
 */
class ToxWhitespaceStripper
{
public:
    ToxWhitespaceStripper(const OUString&);

    sal_Int32
    GetPositionInStrippedString(sal_Int32 pos) const;

    const OUString&
    GetStrippedString() const { return mStripped;}

private:
    OUString mStripped;
    std::vector<sal_Int32> mNewPositions;
};

} // end namespace sw


#endif /* TOXWHITESPACESTRIPPER_HXX_ */
