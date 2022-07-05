#pragma once
#include <linguistic/lngdllapi.h>
#include <rtl/string.hxx>

namespace linguistic
{
LNG_DLLPUBLIC OString Translate(const OString& rTargetLang, const OString& rAPIUrl,
                                const OString& rAuthKey, const OString& rData);
} // namespace
