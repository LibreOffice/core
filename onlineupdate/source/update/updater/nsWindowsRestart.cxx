/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// This file is not build directly. Instead, it is included in multiple
// shared objects.

#ifdef nsWindowsRestart_cxx
#error "nsWindowsRestart.cxx is not a header file, and must only be included once."
#else
#define nsWindowsRestart_cxx
#endif

#ifdef WNT
#include <windows.h>
#endif
#include <stdint.h>
#include <cassert>
#include <iostream>

#include <shellapi.h>

// Needed for CreateEnvironmentBlock
#include <userenv.h>
#pragma comment(lib, "userenv.lib")

// Adapted from Mozilla's nsCharTraits.h
/*
 * Some macros for converting char16_t (UTF-16) to and from Unicode scalar
 * values.
 *
 * Note that UTF-16 represents all Unicode scalar values up to U+10FFFF by
 * using "surrogate pairs". These consist of a high surrogate, i.e. a code
 * point in the range U+D800 - U+DBFF, and a low surrogate, i.e. a code point
 * in the range U+DC00 - U+DFFF, like this:
 *
 *  U+D800 U+DC00 =  U+10000
 *  U+D800 U+DC01 =  U+10001
 *  ...
 *  U+DBFF U+DFFE = U+10FFFE
 *  U+DBFF U+DFFF = U+10FFFF
 *
 * These surrogate code points U+D800 - U+DFFF are not themselves valid Unicode
 * scalar values and are not well-formed UTF-16 except as high-surrogate /
 * low-surrogate pairs.
 */

#define PLANE1_BASE          uint32_t(0x00010000)
// High surrogates are in the range 0xD800 -- OxDBFF
#define NS_IS_HIGH_SURROGATE(u) ((uint32_t(u) & 0xFFFFFC00) == 0xD800)
// Low surrogates are in the range 0xDC00 -- 0xDFFF
#define NS_IS_LOW_SURROGATE(u)  ((uint32_t(u) & 0xFFFFFC00) == 0xDC00)
// Faster than testing NS_IS_HIGH_SURROGATE || NS_IS_LOW_SURROGATE
#define IS_SURROGATE(u)      ((uint32_t(u) & 0xFFFFF800) == 0xD800)

// Everything else is not a surrogate: 0x000 -- 0xD7FF, 0xE000 -- 0xFFFF

// Extract surrogates from a UCS4 char
// Reference: the Unicode standard 4.0, section 3.9
// Since (c - 0x10000) >> 10 == (c >> 10) - 0x0080 and
// 0xD7C0 == 0xD800 - 0x0080,
// ((c - 0x10000) >> 10) + 0xD800 can be simplified to
#define H_SURROGATE(c) char16_t(char16_t(uint32_t(c) >> 10) + \
                                char16_t(0xD7C0))
// where it's to be noted that 0xD7C0 is not bitwise-OR'd
// but added.

// Since 0x10000 & 0x03FF == 0,
// (c - 0x10000) & 0x03FF == c & 0x03FF so that
// ((c - 0x10000) & 0x03FF) | 0xDC00 is equivalent to
#define L_SURROGATE(c) char16_t(char16_t(uint32_t(c) & uint32_t(0x03FF)) | \
                                 char16_t(0xDC00))

#define IS_IN_BMP(ucs) (uint32_t(ucs) < PLANE1_BASE)
#define UCS2_REPLACEMENT_CHAR char16_t(0xFFFD)

#define UCS_END uint32_t(0x00110000)
#define IS_VALID_CHAR(c) ((uint32_t(c) < UCS_END) && !IS_SURROGATE(c))
#define ENSURE_VALID_CHAR(c) (IS_VALID_CHAR(c) ? (c) : UCS2_REPLACEMENT_CHAR)

// UTF8traits adapted from Mozilla's nsUTF8Utils.h
class UTF8traits
{
public:
  static bool isASCII(char aChar)
  {
    return (aChar & 0x80) == 0x00;
  }
  static bool isInSeq(char aChar)
  {
    return (aChar & 0xC0) == 0x80;
  }
  static bool is2byte(char aChar)
  {
    return (aChar & 0xE0) == 0xC0;
  }
  static bool is3byte(char aChar)
  {
    return (aChar & 0xF0) == 0xE0;
  }
  static bool is4byte(char aChar)
  {
    return (aChar & 0xF8) == 0xF0;
  }
  static bool is5byte(char aChar)
  {
    return (aChar & 0xFC) == 0xF8;
  }
  static bool is6byte(char aChar)
  {
    return (aChar & 0xFE) == 0xFC;
  }
};

// UTF8CharEnumerator adapted from Mozilla's nsUTF8Utils.h
/**
 * Extract the next UCS-4 character from the buffer and return it.  The
 * pointer passed in is advanced to the start of the next character in the
 * buffer.  If non-null, the parameters err and overlong are filled in to
 * indicate that the character was represented by an overlong sequence, or
 * that an error occurred.
 */

class UTF8CharEnumerator
{
public:
  static uint32_t NextChar(const char** aBuffer, const char* aEnd, bool* aErr)
  {
    // If this assert fails check if the buffer is null
    assert(aBuffer && *aBuffer);

    const char* p = *aBuffer;
    *aErr = false;

    if (p >= aEnd) {
      *aErr = true;

      return 0;
    }

    char c = *p++;

    if (UTF8traits::isASCII(c)) {
      *aBuffer = p;
      return c;
    }

    uint32_t ucs4;
    uint32_t minUcs4;
    int32_t state = 0;

    if (!CalcState(c, ucs4, minUcs4, state)) {
      std::cerr << "Not a UTF-8 string. This code should only be used for converting from known UTF-8 strings."
                << '\n';
      *aErr = true;

      return 0;
    }

    while (state--) {
      if (p == aEnd) {
        *aErr = true;

        return 0;
      }

      c = *p++;

      if (!AddByte(c, state, ucs4)) {
        *aErr = true;

        return 0;
      }
    }

    if (ucs4 < minUcs4) {
      // Overlong sequence
      ucs4 = UCS2_REPLACEMENT_CHAR;
    } else if (ucs4 >= 0xD800 &&
               (ucs4 <= 0xDFFF || ucs4 >= UCS_END)) {
      // Surrogates and code points outside the Unicode range.
      ucs4 = UCS2_REPLACEMENT_CHAR;
    }

    *aBuffer = p;
    return ucs4;
  }

private:
  static bool CalcState(char aChar, uint32_t& aUcs4, uint32_t& aMinUcs4,
                        int32_t& aState)
  {
    if (UTF8traits::is2byte(aChar)) {
      aUcs4 = (uint32_t(aChar) << 6) & 0x000007C0L;
      aState = 1;
      aMinUcs4 = 0x00000080;
    } else if (UTF8traits::is3byte(aChar)) {
      aUcs4 = (uint32_t(aChar) << 12) & 0x0000F000L;
      aState = 2;
      aMinUcs4 = 0x00000800;
    } else if (UTF8traits::is4byte(aChar)) {
      aUcs4 = (uint32_t(aChar) << 18) & 0x001F0000L;
      aState = 3;
      aMinUcs4 = 0x00010000;
    } else if (UTF8traits::is5byte(aChar)) {
      aUcs4 = (uint32_t(aChar) << 24) & 0x03000000L;
      aState = 4;
      aMinUcs4 = 0x00200000;
    } else if (UTF8traits::is6byte(aChar)) {
      aUcs4 = (uint32_t(aChar) << 30) & 0x40000000L;
      aState = 5;
      aMinUcs4 = 0x04000000;
    } else {
      return false;
    }

    return true;
  }

  static bool AddByte(char aChar, int32_t aState, uint32_t& aUcs4)
  {
    if (UTF8traits::isInSeq(aChar)) {
      int32_t shift = aState * 6;
      aUcs4 |= (uint32_t(aChar) & 0x3F) << shift;
      return true;
    }

    return false;
  }
};

// ConvertUTF8toUTF16 adapted from Mozilla's nsUTF8Utils.h
/**
 * A character sink (see |copy_string| in nsAlgorithm.h) for converting
 * UTF-8 to UTF-16
 */
class ConvertUTF8toUTF16
{
public:
  typedef char value_type;
  typedef char16_t buffer_type;

  explicit ConvertUTF8toUTF16(buffer_type* aBuffer)
    : mStart(aBuffer), mBuffer(aBuffer), mErrorEncountered(false)
  {
  }

  size_t Length() const
  {
    return mBuffer - mStart;
  }

  bool ErrorEncountered() const
  {
    return mErrorEncountered;
  }

  void write(const value_type* aStart, uint32_t aN)
  {
    if (mErrorEncountered) {
      return;
    }

    // algorithm assumes utf8 units won't
    // be spread across fragments
    const value_type* p = aStart;
    const value_type* end = aStart + aN;
    buffer_type* out = mBuffer;
    for (; p != end /* && *p */;) {
      bool err;
      uint32_t ucs4 = UTF8CharEnumerator::NextChar(&p, end, &err);

      if (err) {
        mErrorEncountered = true;
        mBuffer = out;
        return;
      }

      if (ucs4 >= PLANE1_BASE) {
        *out++ = (buffer_type)H_SURROGATE(ucs4);
        *out++ = (buffer_type)L_SURROGATE(ucs4);
      } else {
        *out++ = ucs4;
      }
    }
    mBuffer = out;
  }

  void write_terminator()
  {
    *mBuffer = buffer_type(0);
  }

private:
  buffer_type* const mStart;
  buffer_type* mBuffer;
  bool mErrorEncountered;
};

/**
 * Get the length that the string will take and takes into account the
 * additional length if the string needs to be quoted and if characters need to
 * be escaped.
 */
static int ArgStrLen(const wchar_t *s)
{
  int i = wcslen(s);
  BOOL hasDoubleQuote = wcschr(s, L'"') != nullptr;
  // Only add doublequotes if the string contains a space or a tab
  BOOL addDoubleQuotes = wcspbrk(s, L" \t") != nullptr;

  if (addDoubleQuotes) {
    i += 2; // initial and final duoblequote
  }

  if (hasDoubleQuote) {
    int backslashes = 0;
    while (*s) {
      if (*s == '\\') {
        ++backslashes;
      } else {
        if (*s == '"') {
          // Escape the doublequote and all backslashes preceding the doublequote
          i += backslashes + 1;
        }

        backslashes = 0;
      }

      ++s;
    }
  }

  return i;
}

/**
 * Copy string "s" to string "d", quoting the argument as appropriate and
 * escaping doublequotes along with any backslashes that immediately precede
 * doublequotes.
 * The CRT parses this to retrieve the original argc/argv that we meant,
 * see STDARGV.C in the MSVC CRT sources.
 *
 * @return the end of the string
 */
static wchar_t* ArgToString(wchar_t *d, const wchar_t *s)
{
  BOOL hasDoubleQuote = wcschr(s, L'"') != nullptr;
  // Only add doublequotes if the string contains a space or a tab
  BOOL addDoubleQuotes = wcspbrk(s, L" \t") != nullptr;

  if (addDoubleQuotes) {
    *d = '"'; // initial doublequote
    ++d;
  }

  if (hasDoubleQuote) {
    int backslashes = 0;
    int i;
    while (*s) {
      if (*s == '\\') {
        ++backslashes;
      } else {
        if (*s == '"') {
          // Escape the doublequote and all backslashes preceding the doublequote
          for (i = 0; i <= backslashes; ++i) {
            *d = '\\';
            ++d;
          }
        }

        backslashes = 0;
      }

      *d = *s;
      ++d; ++s;
    }
  } else {
    wcscpy(d, s);
    d += wcslen(s);
  }

  if (addDoubleQuotes) {
    *d = '"'; // final doublequote
    ++d;
  }

  return d;
}

/**
 * Creates a command line from a list of arguments. The returned
 * string is allocated with "malloc" and should be "free"d.
 *
 * argv is UTF8
 */
wchar_t*
MakeCommandLine(int argc, wchar_t **argv)
{
  int i;
  int len = 0;

  // The + 1 of the last argument handles the allocation for null termination
  for (i = 0; i < argc; ++i)
    len += ArgStrLen(argv[i]) + 1;

  // Protect against callers that pass 0 arguments
  if (len == 0)
    len = 1;

  wchar_t *s = (wchar_t*) malloc(len * sizeof(wchar_t));
  if (!s)
    return nullptr;

  wchar_t *c = s;
  for (i = 0; i < argc; ++i) {
    c = ArgToString(c, argv[i]);
    if (i + 1 != argc) {
      *c = ' ';
      ++c;
    }
  }

  *c = '\0';

  return s;
}

/**
 * Convert UTF8 to UTF16 without using the normal XPCOM goop, which we
 * can't link to updater.exe.
 */
static char16_t*
AllocConvertUTF8toUTF16(const char *arg)
{
  // UTF16 can't be longer in units than UTF8
  int len = strlen(arg);
  char16_t *s = new char16_t[(len + 1) * sizeof(char16_t)];
  if (!s)
    return nullptr;

  ConvertUTF8toUTF16 convert(s);
  convert.write(arg, len);
  convert.write_terminator();
  return s;
}

static void
FreeAllocStrings(int argc, wchar_t **argv)
{
  while (argc) {
    --argc;
    delete [] argv[argc];
  }

  delete [] argv;
}



/**
 * Launch a child process with the specified arguments.
 * @note argv[0] is ignored
 * @note The form of this function that takes char **argv expects UTF-8
 */

BOOL
WinLaunchChild(const wchar_t *exePath,
               int argc, wchar_t **argv,
               HANDLE userToken = nullptr,
               HANDLE *hProcess = nullptr);

BOOL
WinLaunchChild(const wchar_t *exePath,
               int argc, char **argv,
               HANDLE userToken,
               HANDLE *hProcess)
{
  wchar_t** argvConverted = new wchar_t*[argc];
  if (!argvConverted)
    return FALSE;

  for (int i = 0; i < argc; ++i) {
      argvConverted[i] = reinterpret_cast<wchar_t*>(AllocConvertUTF8toUTF16(argv[i]));
    if (!argvConverted[i]) {
      FreeAllocStrings(i, argvConverted);
      return FALSE;
    }
  }

  BOOL ok = WinLaunchChild(exePath, argc, argvConverted, userToken, hProcess);
  FreeAllocStrings(argc, argvConverted);
  return ok;
}

BOOL
WinLaunchChild(const wchar_t *exePath,
               int argc,
               wchar_t **argv,
               HANDLE userToken,
               HANDLE *hProcess)
{
  wchar_t *cl;
  BOOL ok;

  cl = MakeCommandLine(argc, argv);
  if (!cl) {
    return FALSE;
  }

  STARTUPINFOW si = {0};
  si.cb = sizeof(STARTUPINFOW);
  si.lpDesktop = L"winsta0\\Default";
  PROCESS_INFORMATION pi = {0};

  if (userToken == nullptr) {
    ok = CreateProcessW(exePath,
                        cl,
                        nullptr,  // no special security attributes
                        nullptr,  // no special thread attributes
                        FALSE, // don't inherit filehandles
                        0,     // creation flags
                        nullptr,  // inherit my environment
                        nullptr,  // use my current directory
                        &si,
                        &pi);
  } else {
    // Create an environment block for the process we're about to start using
    // the user's token.
    LPVOID environmentBlock = nullptr;
    if (!CreateEnvironmentBlock(&environmentBlock, userToken, TRUE)) {
      environmentBlock = nullptr;
    }

    ok = CreateProcessAsUserW(userToken,
                              exePath,
                              cl,
                              nullptr,  // no special security attributes
                              nullptr,  // no special thread attributes
                              FALSE,    // don't inherit filehandles
                              0,        // creation flags
                              environmentBlock,
                              nullptr,  // use my current directory
                              &si,
                              &pi);

    if (environmentBlock) {
      DestroyEnvironmentBlock(environmentBlock);
    }
  }

  if (ok) {
    if (hProcess) {
      *hProcess = pi.hProcess; // the caller now owns the HANDLE
    } else {
      CloseHandle(pi.hProcess);
    }
    CloseHandle(pi.hThread);
  } else {
    LPVOID lpMsgBuf = nullptr;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  nullptr,
                  GetLastError(),
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR) &lpMsgBuf,
                  0,
                  nullptr);
    wprintf(L"Error restarting: %s\n", lpMsgBuf ? lpMsgBuf : L"(null)");
    if (lpMsgBuf)
      LocalFree(lpMsgBuf);
  }

  free(cl);

  return ok;
}
