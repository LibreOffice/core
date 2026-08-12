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
    ios = import shells/ios.nix { inherit pkgs; };
    android = import shells/android.nix { inherit pkgs; };
    android-arm64-v8a = import shells/android.nix {
      inherit pkgs;
      abis = [ "arm64-v8a" ];
    };
    android-armeabi-v7a = import shells/android.nix {
      inherit pkgs;
      abis = [ "armeabi-v7a" ];
    };
    android-x86 = import shells/android.nix {
      inherit pkgs;
      abis = [ "x86" ];
    };
    android-x86_64 = import shells/android.nix {
      inherit pkgs;
      abis = [ "x86_64" ];
    };
  };
}
