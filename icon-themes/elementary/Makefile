SHELL = /bin/sh

TARGET = images_elementary.zip
DESTDIR =
PREFIX = $(DESTDIR)/usr
LIBDIR = $(PREFIX)/lib/libreoffice/share/config
SHAREDIR = $(PREFIX)/share/libreoffice/share/config

all: images_elementary.zip

images_elementary.zip:
	(cd src; zip -r ../$(TARGET) *)

install: images_elementary.zip
	install -m 644 -D $(TARGET) $(SHAREDIR)/$(TARGET)
	ln -sf $(SHAREDIR)/$(TARGET) $(LIBDIR)/$(TARGET)

uninstall:
	-rm $(LIBDIR)/$(TARGET)
	-rm $(SHAREDIR)/$(TARGET)

clean:
	-rm -f images_elementary.zip
