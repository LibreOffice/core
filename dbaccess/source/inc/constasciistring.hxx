#ifndef _DBASHARED_CONSTASCIISTRING_HXX_
#define _DBASHARED_CONSTASCIISTRING_HXX_

#ifndef CONSTASCII_INCLUDED_INDIRECT
#error "don't include this file directly! use stringconstants.hrc instead!"
#endif

// no namespaces. This file is included from several other files _within_ a namespace.

//============================================================
//= a helper for static ascii pseudo-unicode strings
//============================================================
// string constants
struct ConstAsciiString
{
    const sal_Char* ascii;
    sal_Int32       length;

    inline  operator ::rtl::OUString () const;
    inline  operator const sal_Char* () const { return ascii; }

    inline ConstAsciiString(const sal_Char* _pAsciiZeroTerminated, const sal_Int32 _nLength);
    inline ~ConstAsciiString();

private:
    mutable rtl_uString*    ustring;
};

//------------------------------------------------------------
inline ConstAsciiString::ConstAsciiString(const sal_Char* _pAsciiZeroTerminated, const sal_Int32 _nLength)
    :ascii(_pAsciiZeroTerminated)
    ,length(_nLength)
    ,ustring(NULL)
{
}

//------------------------------------------------------------
inline ConstAsciiString::~ConstAsciiString()
{
    if (ustring)
    {
        rtl_uString_release(ustring);
        ustring = NULL;
    }
}

//------------------------------------------------------------
inline ConstAsciiString::operator ::rtl::OUString () const
{
    if (!ustring)
        rtl_uString_newFromAscii( &ustring, ascii );
    return ::rtl::OUString(ustring);
}

//============================================================

#define DECLARE_CONSTASCII_USTRING( name ) \
    extern const ConstAsciiString name

#define IMPLEMENT_CONSTASCII_USTRING( name, string ) \
    const ConstAsciiString name(string, sizeof(string)-1);


#endif // _DBASHARED_CONSTASCIISTRING_HXX_
