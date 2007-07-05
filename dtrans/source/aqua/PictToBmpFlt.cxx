#include "PictToBmpFlt.hxx"

/* This is a work-around to prevent 'deprecated' warning for 'KillPicture' API
   Hopefully we can get rid of this whole code again when the OOo PICT filter
   are good enough to be used see #i78953 thus this hack would vanish to again.
 */
#include <AvailabilityMacros.h>
#undef DEPRECATED_ATTRIBUTE
#define DEPRECATED_ATTRIBUTE

#include <premac.h>
#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>
#include <postmac.h>


bool PICTtoBMP(com::sun::star::uno::Sequence<sal_Int8>& aPict,
               com::sun::star::uno::Sequence<sal_Int8>& aBmp)
{
  bool result = false;

  ComponentInstance bmpExporter;
  if (OpenADefaultComponent(GraphicsExporterComponentType,
                            kQTFileTypeBMP,
                            &bmpExporter) != noErr)
    {
      return result;
    }

  Handle hPict;
  if (PtrToHand(aPict.getArray(), &hPict, aPict.getLength()) != noErr)
    {
      return result;
    }

  Handle hBmp;
  if ((GraphicsExportSetInputPicture(bmpExporter, (PicHandle)hPict) != noErr) ||
      ((hBmp = NewHandleClear(0)) == NULL))
    {
      CloseComponent(bmpExporter);
      DisposeHandle(hPict);
      return result;
    }

  if ((GraphicsExportSetOutputHandle(bmpExporter, hBmp) == noErr) &&
      (GraphicsExportDoExport(bmpExporter, NULL) == noErr))
    {
      size_t sz = GetHandleSize(hBmp);
      aBmp.realloc(sz);

      HLock(hBmp);
      rtl_copyMemory(aBmp.getArray(), ((sal_Int8*)*hBmp), sz);
      HUnlock(hBmp);

      result = true;
    }

  DisposeHandle(hPict);
  DisposeHandle(hBmp);
  CloseComponent(bmpExporter);

  return result;
}

bool BMPtoPICT(com::sun::star::uno::Sequence<sal_Int8>& aBmp,
               com::sun::star::uno::Sequence<sal_Int8>& aPict)
{
  bool result = false;

  Handle hBmp;
  ComponentInstance pictExporter;
  if ((PtrToHand(aBmp.getArray(), &hBmp, aBmp.getLength()) != noErr))
    {
      return result;
    }

  if (OpenADefaultComponent(GraphicsImporterComponentType,
                            kQTFileTypeBMP,
                            &pictExporter) != noErr)
    {
      DisposeHandle(hBmp);
      return result;
    }

  if (GraphicsImportSetDataHandle(pictExporter, hBmp) != noErr)
    {
      DisposeHandle(hBmp);
      CloseComponent(pictExporter);
      return result;
    }

  PicHandle hPict;
  if (GraphicsImportGetAsPicture(pictExporter, &hPict) == noErr)
    {
      size_t sz = GetHandleSize((Handle)hPict);
      aPict.realloc(sz);

      HLock((Handle)hPict);
      rtl_copyMemory(aPict.getArray(), ((sal_Int8*)*hPict), sz);
      HUnlock((Handle)hPict);

      // Release the data associated with the picture
      // Note: This function is deprecated in Mac OS X
      // 10.4.
      KillPicture(hPict);

      result = true;
    }

  DisposeHandle(hBmp);
  CloseComponent(pictExporter);

  return result;
}
