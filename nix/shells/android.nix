{
  pkgs,
  abis ? [
    "arm64-v8a"
    "armeabi-v7a"
    "x86"
    "x86_64"
  ],
  ...
}:
let
  ndkVersion = "29.0.14206865";
  cmakeVersion = "3.22.1";
  buildToolsVersion = "37.0.0"; # can't use 'latest' because (1) this needs to be the same as the version in build.gradle (2) we need to use this version for our AAPT2 override later in this file
  platformToolsVersion = "latest";
  minPlatformVersion = "35"; # https://endoflife.date/android

  androidPkgs = pkgs.androidenv.composeAndroidPackages {
    inherit minPlatformVersion platformToolsVersion;
    abiVersions = abis;
    includeNDK = true;
    includeSources = true;
    ndkVersions = [
      ndkVersion
    ];
    cmakeVersions = [
      cmakeVersion
    ];
    buildToolsVersions = [
      buildToolsVersion
    ];
  };
  androidSdk = androidPkgs.androidsdk;
  sdkPath = "${androidSdk}/libexec/android-sdk";
  ndkPath = "${sdkPath}/ndk/${ndkVersion}";
  toolchainBinPath = "${ndkPath}/toolchains/llvm/prebuilt/${pkgs.clangStdenv.hostPlatform.parsed.kernel.name}-${pkgs.clangStdenv.hostPlatform.uname.processor}/bin"; # e.g. linux-x86_64
in
pkgs.mkShell.override { stdenv = pkgs.clangStdenv; } {
  packages = with pkgs; [
    androidPkgs.platform-tools
    autoconf
    automake
    bison
    cacert
    curl
    flex
    git
    gperf
    jdk17
    libtool
    libuuid
    meson
    ninja
    nodejs_latest
    perl
    pkg-config
    pkgconf
    python3Packages.lxml
    python3Packages.polib
    unzip
    zip
  ];

  ANDROID_BUILD_TOOLS = "${sdkPath}/build-tools/${buildToolsVersion}";
  ANDROID_HOME = sdkPath;
  ANDROID_NDK_ROOT = ndkPath;
  GRADLE_OPTS = "-Dorg.gradle.project.android.aapt2FromMavenOverride=${sdkPath}/build-tools/${buildToolsVersion}/aapt2";
  STUDIO_GRADLE_JDK = "${pkgs.jdk17}/lib/openjdk";
  TMPDIR = "/tmp";

  shellHook = ''
    unset CC CXX AR LD
    export STRIP=llvm-strip
    export PATH="$PATH:${toolchainBinPath}"
    export LD_LIBRARY_PATH="${pkgs.libgcc}/lib:$LD_LIBRARY_PATH"
  '';
}
