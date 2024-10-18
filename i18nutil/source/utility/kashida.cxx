/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <i18nutil/kashida.hxx>
#include <i18nutil/unicode.hxx>
#include <sal/log.hxx>

namespace
{
/*
   https://www.khtt.net/en/page/1821/the-big-kashida-secret

   the rules of priorities that govern the addition of kashidas in Arabic text
   made ... for ... Explorer 5.5 browser.

   The kashida justification is based on a connection priority scheme that
   decides where kashidas are put automatically.

   This is how the software decides on kashida-inserting priorities:
   1. First it looks for characters with the highest priority in each word,
   which means kashida-extensions will only been used in one position in each
   word. Not more.
   2. The kashida will be connected to the character with the highest priority.
   3. If kashida connection opportunities are found with an equal level of
   priority in one word, the kashida will be placed towards the end of the
   word.

   The priority list of characters and the positioning is as follows:
   1. after a kashida that is manually placed in the text by the user,
   2. after a Seen or Sad (initial and medial form),
   3. before the final form of Taa Marbutah, Haa, Dal,
   4. before the final form of Alef, Tah Lam, Kaf and Gaf,
   5. before the preceding medial Baa of Ra, Ya and Alef Maqsurah,
   6. before the final form of Waw, Ain, Qaf and Fa,
   7. before the final form of other characters that can be connected.
*/

/*
   The LibreOffice implementation modifies the above rules, as follows:

   - tdf#65344: Kashida must not be inserted before the final form of Yeh, unless
                preceded by an initial or medial Seen.
   - tdf#163105: As a last resort, use the last valid insertion position from VCL.
*/

#define IS_JOINING_GROUP(c, g) (u_getIntPropertyValue((c), UCHAR_JOINING_GROUP) == U_JG_##g)
#define isAinChar(c) IS_JOINING_GROUP((c), AIN)
#define isAlefChar(c) IS_JOINING_GROUP((c), ALEF)
#define isDalChar(c) IS_JOINING_GROUP((c), DAL)
#define isFehChar(c) (IS_JOINING_GROUP((c), FEH) || IS_JOINING_GROUP((c), AFRICAN_FEH))
#define isGafChar(c) IS_JOINING_GROUP((c), GAF)
#define isHehChar(c) IS_JOINING_GROUP((c), HEH)
#define isKafChar(c) IS_JOINING_GROUP((c), KAF)
#define isLamChar(c) IS_JOINING_GROUP((c), LAM)
#define isQafChar(c) (IS_JOINING_GROUP((c), QAF) || IS_JOINING_GROUP((c), AFRICAN_QAF))
#define isRehChar(c) IS_JOINING_GROUP((c), REH)
#define isTahChar(c) IS_JOINING_GROUP((c), TAH)
#define isTehMarbutaChar(c) IS_JOINING_GROUP((c), TEH_MARBUTA)
#define isWawChar(c) IS_JOINING_GROUP((c), WAW)
#define isSeenOrSadChar(c) (IS_JOINING_GROUP((c), SAD) || IS_JOINING_GROUP((c), SEEN))

// Beh and characters that behave like Beh in medial form.
bool isBehChar(sal_Unicode cCh)
{
    bool bRet = false;
    switch (u_getIntPropertyValue(cCh, UCHAR_JOINING_GROUP))
    {
        case U_JG_BEH:
        case U_JG_NOON:
        case U_JG_AFRICAN_NOON:
        case U_JG_NYA:
        case U_JG_YEH:
        case U_JG_FARSI_YEH:
        case U_JG_BURUSHASKI_YEH_BARREE:
            bRet = true;
            break;
        default:
            bRet = false;
            break;
    }

    return bRet;
}

// Yeh and characters that behave like Yeh in final form.
bool isYehChar(sal_Unicode cCh)
{
    bool bRet = false;
    switch (u_getIntPropertyValue(cCh, UCHAR_JOINING_GROUP))
    {
        case U_JG_YEH:
        case U_JG_FARSI_YEH:
        case U_JG_YEH_BARREE:
        case U_JG_BURUSHASKI_YEH_BARREE:
        case U_JG_YEH_WITH_TAIL:
            bRet = true;
            break;
        default:
            bRet = false;
            break;
    }

    return bRet;
}

bool isTransparentChar(sal_Unicode cCh)
{
    return u_getIntPropertyValue(cCh, UCHAR_JOINING_TYPE) == U_JT_TRANSPARENT;
}

// Checks if cCh + cNectCh builds a ligature (used for Kashidas)
bool isLigature(sal_Unicode cCh, sal_Unicode cNextCh)
{
    // Lam + Alef
    return (isLamChar(cCh) && isAlefChar(cNextCh));
}

// Checks if cCh is connectable to cPrevCh (used for Kashidas)
bool CanConnectToPrev(sal_Unicode cCh, sal_Unicode cPrevCh)
{
    const int32_t nJoiningType = u_getIntPropertyValue(cPrevCh, UCHAR_JOINING_TYPE);
    bool bRet = nJoiningType != U_JT_RIGHT_JOINING && nJoiningType != U_JT_NON_JOINING;

    // check for ligatures cPrevChar + cChar
    if (bRet)
        bRet = !isLigature(cPrevCh, cCh);

    return bRet;
}

bool isSyriacChar(sal_Unicode cCh)
{
    return u_getIntPropertyValue(cCh, UCHAR_SCRIPT) == USCRIPT_SYRIAC;
}

bool isArabicChar(sal_Unicode cCh)
{
    return u_getIntPropertyValue(cCh, UCHAR_SCRIPT) == USCRIPT_ARABIC;
}

std::optional<i18nutil::KashidaPosition>
GetWordKashidaPositionArabic(const OUString& rWord, const std::vector<bool>& pValidPositions)
{
    sal_Int32 nIdx = 0;
    sal_Int32 nPrevIdx = 0;
    sal_Int32 nKashidaPos = -1;
    sal_Unicode cCh = 0;
    sal_Unicode cPrevCh = 0;

    int nPriorityLevel = 8; // 0..7 = level found, 8 not found

    sal_Int32 nWordLen = rWord.getLength();

    // ignore trailing vowel chars
    while (nWordLen && isTransparentChar(rWord[nWordLen - 1]))
    {
        --nWordLen;
    }

    auto fnTryInsertBefore
        = [&rWord, &nIdx, &nPrevIdx, &nKashidaPos, &nPriorityLevel, &nWordLen,
           &pValidPositions](sal_Int32 nNewPriority, bool bIgnoreFinalYeh = false) {
              // Exclusions:

              // tdf#163105: Do not insert kashida if the position is invalid
              if (!pValidPositions.empty() && !pValidPositions[nPrevIdx])
              {
                  return;
              }

              // #i98410#: prevent ZWNJ expansion
              if (rWord[nPrevIdx] == 0x200C || rWord[nPrevIdx + 1] == 0x200C)
              {
                  return;
              }

              // tdf#65344: Do not insert kashida before a final Yeh
              if (!bIgnoreFinalYeh && nIdx == (nWordLen - 1) && isYehChar(rWord[nIdx]))
              {
                  return;
              }

              nKashidaPos = nPrevIdx;
              nPriorityLevel = nNewPriority;
          };

    while (nIdx < nWordLen)
    {
        cCh = rWord[nIdx];

        // 1. Priority:
        // after user inserted kashida
        if (0x640 == cCh)
        {
            // Always respect a user-inserted kashida
            nKashidaPos = nIdx;
            nPriorityLevel = 0;
        }

        // 2. Priority:
        // after a Seen or Sad
        if (nPriorityLevel >= 1)
        {
            if (isSeenOrSadChar(cPrevCh))
            {
                fnTryInsertBefore(1, /*bIgnoreFinalYeh*/ true);
            }
        }

        // 3. Priority:
        // before final form of Teh Marbuta, Heh, Dal
        if (nPriorityLevel >= 2 && nIdx > 0)
        {
            // Teh Marbuta (right joining)
            // Dal (right joining) final form may appear in the middle of word
            // Heh (dual joining) only at end of word
            if (isTehMarbutaChar(cCh) || isDalChar(cCh) || (isHehChar(cCh) && nIdx == nWordLen - 1))
            {
                SAL_WARN_IF(0 == cPrevCh, "i18n", "No previous character");
                // check if character is connectable to previous character,
                if (CanConnectToPrev(cCh, cPrevCh))
                {
                    fnTryInsertBefore(2);
                }
            }
        }

        // 4. Priority:
        // before final form of Alef, Tah, Lam, Kaf or Gaf
        if (nPriorityLevel >= 3 && nIdx > 0)
        {
            // Alef (right joining) final form may appear in the middle of word
            // Lam, Tah, Kaf (all dual joining) only at end of word
            if (isAlefChar(cCh)
                || ((isLamChar(cCh) || isTahChar(cCh) || isKafChar(cCh) || isGafChar(cCh))
                    && nIdx == nWordLen - 1))
            {
                SAL_WARN_IF(0 == cPrevCh, "i18n", "No previous character");
                // check if character is connectable to previous character,
                if (CanConnectToPrev(cCh, cPrevCh))
                {
                    fnTryInsertBefore(3);
                }
            }
        }

        // 5. Priority:
        // before medial Beh-like
        if (nPriorityLevel >= 4 && nIdx > 0 && nIdx < nWordLen - 1)
        {
            if (isBehChar(cCh))
            {
                // check if next character is Reh or Yeh-like
                sal_Unicode cNextCh = rWord[nIdx + 1];
                if (isRehChar(cNextCh) || isYehChar(cNextCh))
                {
                    SAL_WARN_IF(0 == cPrevCh, "i18n", "No previous character");
                    // check if character is connectable to previous character,
                    if (CanConnectToPrev(cCh, cPrevCh))
                    {
                        fnTryInsertBefore(4);
                    }
                }
            }
        }

        // 6. Priority:
        // before the final form of Waw, Ain, Qaf and Feh
        if (nPriorityLevel >= 5 && nIdx > 0)
        {
            // Wav (right joining) final form may appear in the middle of word
            // Ain, Qaf, Feh (all dual joining) only at end of word
            if (isWawChar(cCh)
                || ((isAinChar(cCh) || isQafChar(cCh) || isFehChar(cCh)) && nIdx == nWordLen - 1))
            {
                SAL_WARN_IF(0 == cPrevCh, "i18n", "No previous character");
                // check if character is connectable to previous character,
                if (CanConnectToPrev(cCh, cPrevCh))
                {
                    fnTryInsertBefore(5);
                }
            }
        }

        // 7. Other connecting possibilities
        if (nPriorityLevel >= 6 && nIdx > 0)
        {
            // Reh, Zain (right joining) final form may appear in the middle of word
            // All others except Yeh - only at end of word
            if (isRehChar(cCh) || (isArabicChar(cCh) && nIdx == nWordLen - 1))
            {
                SAL_WARN_IF(0 == cPrevCh, "i18n", "No previous character");
                // check if character is connectable to previous character,
                if (CanConnectToPrev(cCh, cPrevCh))
                {
                    fnTryInsertBefore(6);
                }
            }
        }

        // 8. Try any valid position
        if (nPriorityLevel >= 7 && nIdx > 0 && isArabicChar(cPrevCh) && isArabicChar(cCh)
            && !pValidPositions.empty())
        {
            fnTryInsertBefore(7);
        }

        // Do not consider vowel marks when checking if a character
        // can be connected to previous character.
        if (!isTransparentChar(cCh))
        {
            cPrevCh = cCh;
            nPrevIdx = nIdx;
        }

        ++nIdx;
    } // end of current word

    if (-1 != nKashidaPos)
    {
        return i18nutil::KashidaPosition{ nKashidaPos };
    }

    return std::nullopt;
}

std::optional<i18nutil::KashidaPosition>
GetWordKashidaPositionSyriac(const OUString& rWord, const std::vector<bool>& pValidPositions)
{
    sal_Int32 nWordLen = rWord.getLength();

    // Search for a user-inserted kashida
    for (sal_Int32 i = nWordLen - 1; i >= 0; --i)
    {
        if (0x640 == rWord[i])
        {
            return i18nutil::KashidaPosition{ i };
        }
    }

    // Always insert kashida from the outside-in:
    // - First, work from the end of the word toward the midpoint
    // - Then, work from the beginning of the word toward the midpoint

    sal_Int32 nWordMidpoint = nWordLen / 2;

    auto fnPositionValid = [&pValidPositions, &rWord](sal_Int32 nIdx) {
        // Exclusions:

        // tdf#163105: Do not insert kashida if the position is invalid
        if (!pValidPositions.empty() && !pValidPositions[nIdx])
        {
            return false;
        }

        sal_Unicode cCh = rWord[nIdx];
        return isSyriacChar(cCh);
    };

    // End to midpoint
    for (sal_Int32 i = nWordLen - 2; i > nWordMidpoint; --i)
    {
        if (fnPositionValid(i))
        {
            return i18nutil::KashidaPosition{ i };
        }
    }

    // Beginning to midpoint
    for (sal_Int32 i = 0; i <= nWordMidpoint; ++i)
    {
        if (fnPositionValid(i))
        {
            return i18nutil::KashidaPosition{ i };
        }
    }

    return std::nullopt;
}
}

std::optional<i18nutil::KashidaPosition>
i18nutil::GetWordKashidaPosition(const OUString& rWord, const std::vector<bool>& pValidPositions)
{
    sal_Int32 nWordLen = rWord.getLength();

    SAL_WARN_IF(!pValidPositions.empty() && pValidPositions.size() != static_cast<size_t>(nWordLen),
                "i18n", "Kashida valid position array wrong size");

    for (sal_Int32 nIdx = 0; nIdx < nWordLen; ++nIdx)
    {
        auto cCh = rWord[nIdx];

        if (isSyriacChar(cCh))
        {
            // This word contains Syriac characters.
            return GetWordKashidaPositionSyriac(rWord, pValidPositions);
        }
    }

    return GetWordKashidaPositionArabic(rWord, pValidPositions);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
