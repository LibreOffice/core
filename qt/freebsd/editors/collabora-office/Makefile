PORTNAME=	collabora-office
DISTVERSION=	26.04.4.0
CATEGORIES=	editors
MASTER_SITES=	https://dev-www.libreoffice.org/src/:src \
		https://dev-www.libreoffice.org/extern/:extern \
		https://github.com/antijingoist/opendyslexic/releases/download/v0.91.12/:dyslexic \
		https://pocoproject.org/releases/poco-1.15.3/:poco
# The engine builds these externals itself; everything else the CPFreeBSDQt
# distro-config takes from ports. Regenerate the list from a configured
# engine tree with "gmake -f Makefile.fetch fetch-names" after a bump of
# engine/download.lst.
DISTFILES=	${DISTNAME}${EXTRACT_SUFX} \
		${ENGINE_SRC_TARBALLS:S/$/:src/} \
		f543e6e2d7275557a839a164941c0a86e5f2c3f2a0042bfc434c88c6dde9e140-opens___.ttf:extern \
		opendyslexic-0.910.12-rc2-2019.10.17.zip:dyslexic \
		poco-1.15.3-all.tar.bz2:poco
DIST_SUBDIR=	collabora-office
EXTRACT_ONLY=	${DISTNAME}${EXTRACT_SUFX}

MAINTAINER=	andras.timar@collabora.com
COMMENT=	Collabora Office desktop application
WWW=		https://www.collaboraoffice.com/

LICENSE=	MPL20

ONLY_FOR_ARCHS=	aarch64 amd64

BUILD_DEPENDS=	bash:shells/bash \
		cppunit-config:devel/cppunit \
		fast_float>=0:math/fast_float \
		frozen>=1:devel/frozen \
		gpatch:devel/patch \
		gperf>=3.1:devel/gperf \
		mdds>=3.0:devel/mdds \
		nasm:devel/nasm \
		node:www/node \
		npm:www/npm \
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
		libclucene-core.so:textproc/clucene \
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

USES=		autoreconf:build bison compiler:c++20-lang desktop-file-utils \
		gettext-tools gmake gnome ninja:build perl5 pkgconfig python \
		qt:6 shebangfix sqlite ssl
USE_GNOME=	libxml2 libxslt
USE_QT=		base tools:build webchannel webengine websockets

USE_GITHUB=	yes
GH_ACCOUNT=	CollaboraOnline
GH_PROJECT=	online.mirror
GH_TAGNAME=	005b94a15d61e8c307bfc48f9168c9c688233dd7

SHEBANG_GLOB=	*.py
BINARY_ALIAS=	make=gmake
# The engine refuses to build as root unless it sees a container marker.
# poudriere builds as root inside a jail.
MAKE_ENV+=	container=port

ENGINE_WRKSRC=	${WRKSRC}/engine
ENGINE_INST=	${ENGINE_WRKSRC}/inst
LO_PATH=	${PREFIX}/lib/collaboraoffice

# --with-lang overrides the shipped language list: the translations live in a
# separate repository that is not part of this distfile yet.
ENGINE_CONFIGURE_ARGS=	--with-distro=CPFreeBSDQt \
			--prefix=${ENGINE_INST} \
			--with-external-tar=${DISTDIR}/${DIST_SUBDIR} \
			--disable-fetch-external \
			--with-parallelism=${MAKE_JOBS_NUMBER} \
			--with-os-version=${OSVERSION} \
			--with-lang=en-US

APP_CONFIGURE_ARGS=	--prefix=${PREFIX} \
			--enable-qtapp \
			--with-lo-path=${LO_PATH} \
			--with-lokit-path=${ENGINE_WRKSRC}/include \
			--with-lo-builddir=${ENGINE_WRKSRC} \
			--with-vendor="Collabora Productivity Limited" \
			--with-app-name="Collabora Office" \
			--with-info-url=https://collaboraoffice.com/ \
			MAKE=${MAKE_CMD}

ENGINE_SRC_TARBALLS=	1725634df4bb3dcb1b2c91a6175f8789-GentiumBasic_1102.zip \
			2024-09-19-karla-main.zip \
			33e1e61fab06a547851ed308b4ffef42-dejavu-fonts-ttf-2.37.zip \
			368f114c078f94214a308a74c7e991bc-crosextrafonts-20130214.tar.gz \
			Agdasima-2.002.zip \
			Amiri-1.001.zip \
			Bacasime_Antique-2.000.zip \
			Belanosima-2.000.zip \
			Caprasimo-1.001.zip \
			Lugrasimo-1.001.zip \
			Lumanosimo-1.010.zip \
			Lunasima-2.009.zip \
			NotoKufiArabic-v2.110.zip \
			NotoNaskhArabic-v2.021.zip \
			NotoSans-v2.015.zip \
			NotoSansArabic-v2.010.zip \
			NotoSansArmenian-v2.008.zip \
			NotoSansGeorgian-v2.005.zip \
			NotoSansHebrew-v3.001.zip \
			NotoSansLao-v2.003.zip \
			NotoSansLisu-v2.102.zip \
			NotoSerif-v2.015.zip \
			NotoSerifArmenian-v2.008.zip \
			NotoSerifGeorgian-v2.003.zip \
			NotoSerifHebrew-v2.004.zip \
			NotoSerifLao-v2.003.zip \
			Open_Sans.zip \
			ReemKufi-2.0.zip \
			Scheherazade-2.100.zip \
			afdko-5.0.1.tar.gz \
			alef-1.001.tar.gz \
			antlr4-cpp-runtime-4.13.2-source.zip \
			c74b7223abe75949b4af367942d96c7a-crosextrafonts-carlito-20130920.tar.gz \
			cairo-1.18.4.tar.xz \
			dragonbox-1.1.3.tar.gz \
			e7a384790b13c29113e22e596ade9687-LinLibertineG-20120116.zip \
			edc4d741888bc0d38e32dbaa17149596-source-sans-pro-2.010R-ro-1.065R-it.tar.gz \
			liberation-fonts-ttf-2.1.5.tar.gz \
			liberation-narrow-fonts-ttf-1.07.6.tar.gz \
			libjpeg-turbo-3.1.3.tar.gz \
			libre-hebrew-1.0.tar.gz \
			meson-1.8.3.tar.gz \
			pdfium-7988.tar.bz2 \
			phc-winner-argon2-20190702.tar.gz \
			pixman-0.46.4.tar.xz \
			quickjs-v0.14.0.tar.gz \
			zxcvbn-c-2.6.tar.gz

do-configure:
	cd ${ENGINE_WRKSRC} && ${SETENV} ${MAKE_ENV} ./autogen.sh ${ENGINE_CONFIGURE_ARGS}

do-build:
	cd ${ENGINE_WRKSRC} && ${SETENV} ${MAKE_ENV} ${MAKE_CMD} && \
		${SETENV} ${MAKE_ENV} ${MAKE_CMD} distro-pack-install
	cd ${WRKSRC} && ${SETENV} ${MAKE_ENV} ./autogen.sh && \
		${SETENV} ${MAKE_ENV} ${CONFIGURE_ENV} ./configure ${APP_CONFIGURE_ARGS} && \
		${SETENV} ${MAKE_ENV} ${MAKE_CMD} ${_MAKE_JOBS}

do-install:
	${MKDIR} ${STAGEDIR}${PREFIX}/lib
	${CP} -a ${ENGINE_INST}/lib/collaboraoffice ${STAGEDIR}${PREFIX}/lib/
	cd ${WRKSRC} && ${SETENV} ${MAKE_ENV} ${MAKE_CMD} DESTDIR=${STAGEDIR} install

post-install:
	${FIND} ${STAGEDIR}${LO_PATH} -type f \
		\( -name '*.so' -o -name '*.so.*' -o -name '*.bin' \) \
		-exec ${STRIP_CMD} {} +
	${STRIP_CMD} ${STAGEDIR}${PREFIX}/bin/coda-qt

.include <bsd.port.mk>
