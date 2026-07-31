let
  pins = import ./npins;

  pkgs = import pins.nixpkgs {
    config.allowUnfree = true;
  };
in {
  shells = {
    ios = import shells/ios.nix { inherit pkgs; };
  };
}
