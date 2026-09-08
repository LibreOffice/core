let
  pins = import ./npins;

  pkgs = import pins.nixpkgs {
    config = {
      allowUnfree = true;
      android_sdk.accept_license = true;
    };
  };
in
{
  shells = {
    android = import shells/android.nix { inherit pkgs; };
    android-arm64-v8a = import shells/android.nix {
      inherit pkgs;
      abis = [ "arm64-v8a" ];
    };
    android-armeabi-v7a = import shells/android.nix {
      inherit pkgs;
      abis = [ "armeabi-v7a" ];
    };
    android-dev = import shells/android.nix {
      inherit pkgs;
      dev = true;
    };
    android-x86 = import shells/android.nix {
      inherit pkgs;
      abis = [ "x86" ];
    };
    android-x86_64 = import shells/android.nix {
      inherit pkgs;
      abis = [ "x86_64" ];
    };
    ios = import shells/ios.nix { inherit pkgs; };
    online = import shells/online.nix { inherit pkgs; };
  };
}
