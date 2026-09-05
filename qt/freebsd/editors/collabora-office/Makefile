PORTNAME=	collabora-office
DISTVERSION=	26.04.3-2
CATEGORIES=	editors
# The source tarball is made by scripts/make-source-tarball.sh from the
# release tag and published next to the branding tarball.
MASTER_SITES=	https://www.collaboraoffice.com/downloads/collabora-office/src/ \
		https://www.collaboraoffice.com/downloads/collabora-office-brand/:brand \
		https://dev-www.libreoffice.org/src/:src \
		https://dev-www.libreoffice.org/extern/:extern \
		https://pocoproject.org/releases/poco-1.15.3/:poco
# The engine builds these externals itself; everything else the CPFreeBSDQt
# distro-config takes from ports. Regenerate the list from a configured
# engine tree with "gmake -f Makefile.fetch fetch-names" after a bump of
# engine/download.lst.
DISTFILES=	${DISTNAME}${EXTRACT_SUFX} \
		collabora-office-brand-${BRAND_VERSION}.tar.gz:brand \
		${ENGINE_SRC_TARBALLS:S/$/:src/} \
		f543e6e2d7275557a839a164941c0a86e5f2c3f2a0042bfc434c88c6dde9e140-opens___.ttf:extern \
		poco-1.15.3-all.tar.bz2:poco
DIST_SUBDIR=	collabora-office
EXTRACT_ONLY=	${DISTNAME}${EXTRACT_SUFX} \
		collabora-office-brand-${BRAND_VERSION}.tar.gz

MAINTAINER=	releng@collaboraoffice.com
COMMENT=	Collabora Office desktop application
WWW=		https://www.collaboraoffice.com/

LICENSE=	MPL20

ONLY_FOR_ARCHS=	aarch64 amd64

# clucene is only checked by configure; nothing links it while help is disabled
BUILD_DEPENDS=	bash:shells/bash \
		cppunit-config:devel/cppunit \
		${LOCALBASE}/libdata/pkgconfig/libclucene-core.pc:textproc/clucene \
		fast_float>=0:math/fast_float \
		frozen>=1:devel/frozen \
		gpatch:devel/patch \
		gperf>=3.1:devel/gperf \
		mdds>=3.0:devel/mdds \
		nasm:devel/nasm \
		node:www/node \
		npm:www/npm \
		rsync:net/rsync \
		zip:archivers/zip \
		${LOCALBASE}/include/glm/glm.hpp:math/glm \
		${LOCALBASE}/include/sane/sane.h:graphics/sane-backends \
		${LOCALBASE}/include/sqlext.h:databases/unixODBC \
		${PYTHON_PKGNAMEPREFIX}lxml>0:devel/py-lxml@${PY_FLAVOR} \
		${PYTHON_PKGNAMEPREFIX}polib>0:devel/py-polib@${PY_FLAVOR}
LIB_DEPENDS=	libCoinMP.so:math/coinmp \
		libZXing.so:textproc/zxing-cpp \
		libabsl_strings.so:devel/abseil \
		libabw-0.1.so:textproc/libabw \
		libboost_date_time.so:devel/boost-libs \
		libcdr-0.1.so:graphics/libcdr01 \
		libcmis-0.6.so:net/libcmis \
		libcurl.so:ftp/curl \
		libe-book-0.1.so:textproc/libe-book \
		libepubgen-0.1.so:textproc/libepubgen \
		libetonyek-0.1.so:graphics/libetonyek01 \
		libexpat.so:textproc/expat2 \
		libexttextcat-2.0.so:textproc/libexttextcat \
		libfontconfig.so:x11-fonts/fontconfig \
		libfreehand-0.1.so:graphics/libfreehand \
		libfreetype.so:print/freetype2 \
		libgraphite2.so:graphics/graphite2 \
		libharfbuzz-icu.so:print/harfbuzz-icu \
		libharfbuzz.so:print/harfbuzz \
		libhunspell-1.7.so:textproc/hunspell \
		libhyphen.so:textproc/hyphen \
		libicuuc.so:devel/icu \
		liblangtag.so:devel/liblangtag \
		liblcms2.so:graphics/lcms2 \
		libmd4c.so:textproc/md4c \
		libmspub-0.1.so:print/libmspub01 \
		libmwaw-0.3.so:textproc/libmwaw03 \
		libmythes-1.2.so:textproc/mythes \
		libnss3.so:security/nss \
		libnumbertext-1.0.so:textproc/libnumbertext \
		libodfgen-0.1.so:textproc/libodfgen01 \
		libopenblas.so:math/openblas \
		libopenjp2.so:graphics/openjpeg \
		liborcus-0.21.so:devel/liborcus \
		libpagemaker-0.0.so:print/libpagemaker \
		libpng.so:graphics/png \
		libqxp-0.0.so:textproc/libqxp \
		librdf.so:textproc/redland \
		librevenge-0.0.so:textproc/librevenge \
		libstaroffice-0.0.so:textproc/libstaroffice \
		libtiff.so:graphics/tiff \
		libvisio-0.1.so:textproc/libvisio01 \
		libwebp.so:graphics/webp \
		libwpd-0.10.so:textproc/libwpd010 \
		libwpg-0.3.so:graphics/libwpg03 \
		libwps-0.4.so:textproc/libwps \
		libxmlsec1-nss.so:security/xmlsec1 \
		libzmf-0.0.so:graphics/libzmf \
		libzstd.so:archivers/zstd

# The engine is built without its bundled fonts; these packages provide the
# families documents most often expect.
RUN_DEPENDS=	${LOCALBASE}/share/fonts/Alef/Alef-Bold.ttf:x11-fonts/alef \
		${LOCALBASE}/share/fonts/Caladea/Caladea-Bold.ttf:x11-fonts/crosextrafonts-caladea-ttf \
		${LOCALBASE}/share/fonts/Carlito/Carlito-Bold.ttf:x11-fonts/crosextrafonts-carlito-ttf \
		${LOCALBASE}/share/fonts/dejavu/DejaVuSans.ttf:x11-fonts/dejavu \
		${LOCALBASE}/share/fonts/Gentium/Gentium-Bold.ttf:x11-fonts/gentium \
		${LOCALBASE}/share/fonts/Liberation/LiberationMono-Bold.ttf:x11-fonts/liberation-fonts-ttf \
		${LOCALBASE}/share/fonts/LinLibertineG/LinLibertine_DR_G.ttf:x11-fonts/linuxlibertine-g \
		${LOCALBASE}/share/fonts/noto/NotoKufiArabic-Black.ttf:x11-fonts/noto-kufi-arabic \
		${LOCALBASE}/share/fonts/noto/NotoNaskhArabic-Bold.ttf:x11-fonts/noto-naskh-arabic \
		${LOCALBASE}/share/fonts/noto/NotoSans-Black.ttf:x11-fonts/noto-sans \
		${LOCALBASE}/share/fonts/noto/NotoSansHebrew-Black.ttf:x11-fonts/noto-sans-hebrew \
		${LOCALBASE}/share/fonts/noto/NotoSerif-Black.ttf:x11-fonts/noto-serif \
		${LOCALBASE}/share/fonts/opensans/OpenSans-Bold.ttf:x11-fonts/opensans-ttf \
		${LOCALBASE}/share/fonts/SourceSans/SourceSans3-Black.ttf:x11-fonts/source-sans-ttf

USES=		autoreconf:build bison compiler:c++20-lang desktop-file-utils \
		gettext-tools gmake gnome ninja:build perl5 pkgconfig python \
		qt:6 shebangfix ssl tar:xz
USE_GNOME=	libxml2 libxslt
# configure checks for Qt WebSockets, but the app does not end up linked to it
USE_QT=		base tools:build webchannel webengine websockets:build

SHEBANG_GLOB=	*.py
BINARY_ALIAS=	make=gmake
# The engine refuses to build as root unless it sees a container marker.
# poudriere builds as root inside a jail.
MAKE_ENV+=	container=port

BRAND_VERSION=	26.04.2.3
BRAND_WRKSRC=	${WRKDIR}/collabora-office-brand
ENGINE_WRKSRC=	${WRKSRC}/engine
# The engine's distro-pack-install puts it under ${PREFIX}/lib/collaboraoffice,
# which is what the app is configured with as --with-lo-path.
LO_PATH=	${PREFIX}/lib/collaboraoffice

ENGINE_CONFIGURE_ARGS=	--with-distro=CPFreeBSDQt \
			--prefix=${PREFIX} \
			--with-external-tar=${DISTDIR}/${DIST_SUBDIR} \
			--disable-fetch-external \
			--with-parallelism=${MAKE_JOBS_NUMBER} \
			--with-os-version=${OSVERSION}

APP_CONFIGURE_ARGS=	--prefix=${PREFIX} \
			--enable-qtapp \
			--with-lo-path=${LO_PATH} \
			--with-lokit-path=${ENGINE_WRKSRC}/include \
			--with-lo-builddir=${ENGINE_WRKSRC} \
			--with-vendor="Collabora Productivity Limited" \
			--with-app-name="Collabora Office" \
			--with-info-url=https://collaboraoffice.com/ \
			--with-app-branding=${BRAND_WRKSRC} \
			MAKE=${MAKE_CMD}

ENGINE_SRC_TARBALLS=	afdko-5.0.1.tar.gz \
			antlr4-cpp-runtime-4.13.2-source.zip \
			cairo-1.18.4.tar.xz \
			dragonbox-1.1.3.tar.gz \
			libjpeg-turbo-3.1.3.tar.gz \
			meson-1.8.3.tar.gz \
			pdfium-7988.tar.bz2 \
			phc-winner-argon2-20190702.tar.gz \
			pixman-0.46.4.tar.xz \
			quickjs-v0.14.0.tar.gz \
			zxcvbn-c-2.6.tar.gz

do-configure:
	cd ${ENGINE_WRKSRC} && ${SETENV} ${MAKE_ENV} ./autogen.sh ${ENGINE_CONFIGURE_ARGS}

do-build:
	cd ${ENGINE_WRKSRC} && ${SETENV} ${MAKE_ENV} ${MAKE_CMD}
	cd ${WRKSRC} && ${SETENV} ${MAKE_ENV} ./autogen.sh && \
		${SETENV} ${MAKE_ENV} ${CONFIGURE_ENV} ./configure ${APP_CONFIGURE_ARGS} && \
		${SETENV} ${MAKE_ENV} ${MAKE_CMD} ${_MAKE_JOBS}

do-install:
	cd ${ENGINE_WRKSRC} && ${SETENV} ${MAKE_ENV} ${MAKE_CMD} DESTDIR=${STAGEDIR} distro-pack-install
	${MKDIR} ${STAGEDIR}${LO_PATH}/share/theme_definitions/online
	${CP} -a ${BRAND_WRKSRC}/online-theme/ ${STAGEDIR}${LO_PATH}/share/theme_definitions/online
	cd ${WRKSRC} && ${SETENV} ${MAKE_ENV} ${MAKE_CMD} DESTDIR=${STAGEDIR} install

post-install:
	${FIND} ${STAGEDIR}${LO_PATH} -type f \
		\( -name '*.so' -o -name '*.so.*' -o -name '*.bin' \) \
		-exec ${STRIP_CMD} {} +
	${STRIP_CMD} ${STAGEDIR}${PREFIX}/bin/coda-qt

.include <bsd.port.mk>
