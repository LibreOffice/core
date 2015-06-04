SHELL = /bin/sh

TARGET = images_elementary.zip
DESTDIR =
PREFIX = $(DESTDIR)/usr
SHAREDIR = $(PREFIX)/share/libreoffice/share/config
LIBDIR = $(PREFIX)/lib/libreoffice/share/config
LIBLINK = ../../../../share/libreoffice/share/config

all: images_elementary.zip

images_elementary.zip:
	(cd src; zip -r ../$(TARGET) *)

install: images_elementary.zip
	install -d $(SHAREDIR)
	install -d $(LIBDIR)
	install -m 644 -D $(TARGET) $(SHAREDIR)/$(TARGET)
	(cd $(LIBDIR); ln -sf $(LIBLINK)/$(TARGET) $(TARGET))

uninstall:
	-rm $(LIBDIR)/$(TARGET)
	-rm $(SHAREDIR)/$(TARGET)

clean:
	-rm -f images_elementary.zip
