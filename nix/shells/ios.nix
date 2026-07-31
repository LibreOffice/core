{ pkgs, ... }:

let
  developerDir = "/Applications/Xcode.app/Contents/Developer";

  allowedXcodeVersions = [ "26.4" ]; # what version does `xcodebuild -version` show?

  xcrun = pkgs.writeShellScriptBin "xcrun" ''
    if [ ! -x /usr/bin/xcrun ]; then
      echo "nix/shells/ios.nix: /usr/bin/xcrun not found; please install XCode (and the iPhoneOS SDK)" >&2
      exit 1
    fi
    exec /usr/bin/xcrun "$@"
  ''; # We shim the system xcrun because the shell's apple SDK tools don't build for iPhones (and the same for XCode installed through nix/etc.)...

  plutil = pkgs.writeShellScriptBin "plutil" ''
    if [ ! -x /usr/bin/plutil ]; then
      echo "nix/shells/ios.nix: /usr/bin/plutil not found; please install XCode (and the iPhoneOS SDK)" >&2
      exit 1
    fi
    exec /usr/bin/plutil "$@"
  ''; # scripts/po_to_strings.py uses plutil - same deal as everything else...

  security = pkgs.writeShellScriptBin "security" ''
    if [ ! -x /usr/bin/security ]; then
      echo "nix/shells/ios.nix: /usr/bin/security not found" >&2
      exit 1
    fi
    exec /usr/bin/security "$@"
  ''; # signing stage unlocks the login keychain via `security`

  rsync = pkgs.writeShellScriptBin "rsync" ''
    if [ ! -x /usr/bin/rsync ]; then
      echo "nix/shells/ios.nix: /usr/bin/rsync not found" >&2
      exit 1
    fi
    exec /usr/bin/rsync "$@"
  ''; # xcodebuild -exportArchive uses rsync --extended-attributes, which is only available on the system rsync

  xcodebuild = pkgs.writeShellScriptBin "xcodebuild" ''
    if [ ! -x /usr/bin/xcodebuild ]; then
      echo "nix/shells/ios.nix: /usr/bin/xcodebuild not found; please install XCode (and the iPhoneOS SDK)" >&2
      exit 1
    fi
    read -r _ version _ < <(/usr/bin/xcodebuild -version)
    for allowed in ${builtins.concatStringsSep " " allowedXcodeVersions}; do
      if [ "$version" = "$allowed" ]; then
        exec /usr/bin/xcodebuild "$@"
      fi
    done
    echo "nix/shells/ios.nix: XCode $version is not pinned (allowed: ${builtins.concatStringsSep ", " allowedXcodeVersions}); update XCode or the pin" >&2
    exit 1
  ''; # configure's SDK check calls xcodebuild... we also hard-fail here if the installed XCode is the wrong version to hopefully improve reproducibility (though it's not gonna be perfect because we aren't able to install XCode through nix)
in
pkgs.mkShell {
  nativeBuildInputs = with pkgs; [
    autoconf
    automake
    bison
    cacert
    curl
    flex
    git
    gperf
    libtool
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

  # configure swaps CC for `xcrun` clang when the env CC is empty, so setting these keeps our nix compiler rather than the host system one
  CC_FOR_BUILD = "cc -isysroot ${pkgs.apple-sdk.sdkroot}";
  CXX_FOR_BUILD = "c++ -isysroot ${pkgs.apple-sdk.sdkroot}";

  shellHook = ''
    export DEVELOPER_DIR=${developerDir}
    export PATH="${xcrun}/bin:${xcodebuild}/bin:${plutil}/bin:${security}/bin:${rsync}/bin:$PATH"
    unset CC CXX LD AR AS NM OBJCOPY OBJDUMP READELF RANLIB STRIP STRINGS SIZE # on the XCode part of the build, we don't want to have overriden these variables using the nix stdenv, the rest of the build searches for stuff in PATH/etc. so should be fine without them...
  ''; # ...and we need to make sure we use the right xcrun and SDK directory, since as pulling in MacOS compilers (as mkShell does) overrides this to the nixpkgs versions
}
